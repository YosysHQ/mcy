#!/usr/bin/env python3

import getopt, sys, os, re, time, signal
import subprocess, sqlite3, uuid, shutil
import types, time

taskidx = 0
taskdb = dict()
running = set()
dbtrace = False
silent_sigpipe = False

def root_path():
    fn = getattr(sys.modules['__main__'], '__file__')
    root_path = os.path.abspath(os.path.dirname(fn))
    return root_path

def sqlite3_connect(chkexist=False):
    if chkexist and not os.path.exists("database/db.sqlite3"):
        print("Project database not found. Run 'mcy init' to initialize the project.")
        exit(1)
    db = sqlite3.connect("database/db.sqlite3")
    if dbtrace:
        db.set_trace_callback(print)
    return db

def exit(rc):
    for task in list(taskdb.values()):
        task.term()
    if len(running):
        db = sqlite3_connect()
        for mut, tst in running:
            db.execute("UPDATE queue SET running = 0 WHERE mutation_id = ? AND test = ?", [mut, tst])
        db.commit()
    sys.exit(rc)

def force_shutdown(signum, frame):
    if (os.name != 'nt' and signum != signal.SIGPIPE) or not silent_sigpipe:
        print("MCY ---- Keyboard interrupt or external termination signal ----", file=sys.stderr, flush=True)
    exit(1)

if os.name == "posix":
    signal.signal(signal.SIGHUP, force_shutdown)
    signal.signal(signal.SIGPIPE, force_shutdown)
signal.signal(signal.SIGINT, force_shutdown)
signal.signal(signal.SIGTERM, force_shutdown)

def usage():
    print()
    print("Usage:")
    print("  mcy [--trace] init [--nosetup]")
    print("  mcy [--trace] reset")
    print("  mcy [--trace] status")
    print("  mcy [--trace] list [--details] [<id_or_tag>..]")
    print("  mcy [--trace] run [-jN] [--reset] [<id>..]")
    print("  mcy [--trace] task [-v] [-k] <test> <id_or_tag>..")
    print("  mcy [--trace] source [-e <encoding>] <filename> [<filename>]")
    print("  mcy [--trace] lcov <filename>")
    print("  mcy [--trace] dash [<source_dir>]")
    print("  mcy [--trace] gui [--src <source_dir>]")
    print("  mcy [--trace] purge")
    print()
    exit(1)

if len(sys.argv) > 1 and sys.argv[1] == "--trace":
    sys.argv[1:] = sys.argv[2:]
    dbtrace = True

if len(sys.argv) < 2:
    usage()

if not os.path.exists("config.mcy"):
    print("config.mcy not found")
    exit(1)

mutate_cfgs = set("""
weight_pq_w weight_pq_b weight_pq_c weight_pq_s
weight_pq_mw weight_pq_mb weight_pq_mc weight_pq_ms
weight_cover pick_cover_prcnt
""".split())

cfg = types.SimpleNamespace()
cfg.opt_size = 20
cfg.opt_tags = None
cfg.opt_seed = None
cfg.opt_mode = None
cfg.mutopts = dict()
cfg.setup = list()
cfg.script = list()
cfg.logic = list()
cfg.report = list()
cfg.tests = dict()
cfg.files = dict()
cfg.select = list()

with open("config.mcy", "r") as f:
    section = None
    sectionarg = None
    linenr = 0
    for line in f:
        linenr += 1

        if line.strip().startswith("#"):
            continue

        match = re.match(r"^\[(.*)\]\s*$", line)
        if match:
            entries = match.group(1).split()
            if len(entries) == 1 and entries[0] in ("options", "script", "setup", "logic", "report", "files"):
                section, sectionarg = entries[0], None
                continue
            if len(entries) == 2 and entries[0] == "test":
                section, sectionarg = entries
                if sectionarg not in cfg.tests:
                    cfg.tests[sectionarg] = types.SimpleNamespace()
                    cfg.tests[sectionarg].maxbatchsize = 1
                    cfg.tests[sectionarg].expect = None
                    cfg.tests[sectionarg].run = None
                continue

        if section == "options":
            entries = line.split()
            if len(entries) == 0:
                continue
            if len(entries) == 2 and entries[0] == "size":
                cfg.opt_size = int(entries[1])
                continue
            if len(entries) == 2 and entries[0] == "mode":
                cfg.opt_mode = entries[1]
                continue
            if len(entries) == 2 and entries[0] in mutate_cfgs:
                cfg.mutopts[entries[0]] = int(entries[1])
                continue
            if len(entries) > 1 and entries[0] == "tags":
                cfg.opt_tags = set(entries[1:])
                continue
            if len(entries) == 2 and entries[0] == "seed":
                cfg.opt_seed = int(entries[1])
                continue
            if len(entries) > 1 and entries[0] == "select":
                cfg.select += entries[1:]
                continue

        if section == "setup":
            cfg.setup.append(line.rstrip())
            continue

        if section == "script":
            cfg.script.append(line.rstrip())
            continue

        if section == "logic":
            cfg.logic.append(line.rstrip())
            continue

        if section == "report":
            cfg.report.append(line.rstrip())
            continue

        if section == "test":
            entries = line.split()
            if len(entries) == 0:
                continue
            if len(entries) == 2 and entries[0] == "maxbatchsize":
                cfg.tests[sectionarg].maxbatchsize = int(entries[1])
                continue
            if len(entries) >= 2 and entries[0] == "expect":
                cfg.tests[sectionarg].expect = entries[1:]
                continue
            if len(entries) >= 2 and entries[0] == "run":
                match = re.match(r"^\s*run\s*(.*\S)\s*$", line)
                cfg.tests[sectionarg].run = match.group(1)
                continue

        if section == "files":
            entries = line.split()
            if len(entries) == 0:
                continue
            if len(entries) == 1:
                cfg.files[entries[0]] = entries[0]
                continue
            if len(entries) == 2:
                cfg.files[entries[0]] = entries[1]
                continue

        print("syntax error in line %d in config.mcy" % linenr)
        exit(1)


######################################################

def xorshift32(x):
    x = (x ^ x << 13) & 0xFFFFFFFF
    x = (x ^ x >> 17) & 0xFFFFFFFF
    x = (x ^ x <<  5) & 0xFFFFFFFF
    return x;

if cfg.opt_seed is None:
    cfg.opt_seed = int(100 * time.time())
    cfg.opt_seed = xorshift32(cfg.opt_seed)
    cfg.opt_seed = xorshift32(cfg.opt_seed)
    cfg.opt_seed = xorshift32(cfg.opt_seed)
    cfg.opt_seed = cfg.opt_seed % 1000000000


######################################################

def update_mutation(db, mid):
    rng_state = xorshift32(xorshift32(mid + cfg.opt_seed))
    rng_state = xorshift32(xorshift32(rng_state))

    db.execute("DELETE FROM queue WHERE mutation_id = ?", [mid])
    db.execute("DELETE FROM tags WHERE mutation_id = ?", [mid])

    class ResultNotReadyException(BaseException):
        def __init__(self, tst):
            self.tst = tst

    def env_result(tst):
        t = tst.split()[0]
        for res, in db.execute("SELECT (result) FROM results WHERE mutation_id = ? AND test = ?", [mid, tst]):
            if cfg.tests[t].expect is not None:
                if not res in cfg.tests[t].expect:
                    raise Exception('Executing %s resulted with %s expecting value(s): %s' % (tst, res, ', '.join(cfg.tests[t].expect)))
            return res
        raise ResultNotReadyException(tst)

    def env_tag(tag):
        if cfg.opt_tags is not None:
            if not tag in cfg.opt_tags:
                raise Exception('Provided tag %s not on of expected: %s' % (tag, ', '.join(cfg.opt_tags)))
        db.execute("INSERT INTO tags (mutation_id, tag) VALUES (?, ?)", [mid, tag])

    def env_rng(n):
        nonlocal rng_state
        rng_state = xorshift32(xorshift32(rng_state))
        return rng_state % n

    gdict = globals().copy()
    gdict["result"] = env_result
    gdict["tag"] = env_tag
    gdict["rng"] = env_rng

    try:
        code = "def __logic__():\n  " + "\n  ".join(cfg.logic) + "\n__logic__()\n"
        exec(code, gdict)
    except ResultNotReadyException as ex:
        db.execute("INSERT INTO queue (mutation_id, test, running) VALUES (?, ?, 0)", [mid, ex.tst])

    db.commit()

def reset_status(db, do_reset=False):
    if do_reset:
        nmutations, = db.execute("SELECT COUNT(*) FROM mutations").fetchone()
        if nmutations < cfg.opt_size:
            print("Adding %d mutations to database." % (cfg.opt_size - nmutations))

            with open("database/mutations2.ys", "w") as f:
                print("read_ilang database/design.il", file=f)
                print(f"mutate -list {cfg.opt_size} -seed {cfg.opt_seed} -none{''.join(' -cfg %s %d' % (k, v) for k, v, in sorted(cfg.mutopts.items()))}{' -mode ' + cfg.opt_mode if cfg.opt_mode else ''} -o database/mutations.txt -s database/sources.txt{' '.join(cfg.select) if len(cfg.select) else ''}", file=f)

            task = Task("yosys -ql database/mutations2.log database/mutations2.ys")
            task.wait()

            with open("database/mutations2.txt", "r") as f_in:
                with open("database/mutations.txt", "a") as f_out:
                    for line in f_in:
                        if not db.execute("SELECT COUNT(*) FROM mutations WHERE mutation = ?", [line.rstrip()]).fetchone()[0]:
                            mid = db.execute("INSERT INTO mutations (mutation) VALUES (?)", [line.rstrip()]).lastrowid
                            print(line.rstrip(), file=f_out)
                            optarray = line.split()
                            skip_next = False
                            for i in range(len(optarray)-1):
                                if skip_next:
                                    skip_next = False
                                elif optarray[i].startswith("-"):
                                    db.execute("INSERT INTO options (mutation_id, opt_type, opt_value) VALUES (?, ?, ?)", [mid, optarray[i][1:], optarray[i+1]])
                                    skip_next = True
                            nmutations += 1
                            if nmutations == cfg.opt_size:
                                break

        shutil.rmtree("tasks", ignore_errors=True)

        for mid, in db.execute("SELECT mutation_id FROM mutations"):
            update_mutation(db, mid)

    cnt, = db.execute("SELECT COUNT(*) FROM results").fetchone()
    print("Database contains %d cached results." % cnt)

    for tst, res, cnt in db.execute("SELECT test, result, COUNT(*) FROM results GROUP BY test, result"):
        print("Database contains %d cached \"%s\" results for \"%s\"." % (cnt, res, tst))

    for tag, cnt in db.execute("SELECT tag, COUNT(*) FROM tags GROUP BY tag"):
        print("Tagged %d mutations as \"%s\"." % (cnt, tag))

    for tst, cnt, rn in db.execute("SELECT test, COUNT(*), SUM(running) FROM queue GROUP BY test"):
        if rn > 0:
            print("Queued %d \"%s\" tests, %d running." % (cnt, tst, rn))
        else:
            print("Queued %d \"%s\" tests." % (cnt, tst))

def print_report(db):
    def env_tags(tag=None):
        if tag is None:
            cnt, = db.execute("SELECT COUNT(DISTINCT mutation_id) FROM tags").fetchone()
            return cnt

        invert = False
        if tag.startswith("!"):
            invert = True
            tag = tag[1:]

        cnt, = db.execute("SELECT COUNT(*) FROM tags WHERE tag = ?", [tag]).fetchone()

        if invert:
            return env_tags() - cnt
        return cnt

    gdict = globals().copy()
    gdict["tags"] = env_tags

    code = "def __report__():\n  " + "\n  ".join(cfg.report) + "\n__report__()\n"
    exec(code, gdict)


######################################################

def wait_tasks(n):
    for task in list(taskdb.values()):
        task.poll()

    while len(taskdb) >= n:
        time.sleep(0.5)  # Ugh!
        for task in list(taskdb.values()):
            task.poll()

class Task:
    def __init__(self, command, callback=None, silent=False, logfilename=None):
        global taskidx
        taskidx += 1
        self.taskidx = taskidx
        self.command = command
        if not silent:
            print(command)
        self.callback = callback
        self.logfilename = logfilename
        self.p = subprocess.Popen(command, shell=True, stdin=subprocess.DEVNULL)
        taskdb[taskidx] = self
        self.running = True

    def poll(self):
        if not self.running:
            return True
        rc = self.p.poll()
        if rc == None:
            return False
        self.running = False
        if self.taskidx in taskdb:
            del taskdb[self.taskidx]
        if rc != 0:
            print("Command '%s' returned non-zero return code %d." % (self.command, rc))
            if self.logfilename is not None:
                print("See '%s' for details." % self.logfilename)
            exit(1)
        if self.callback is not None:
            self.callback()
            self.callback = None
        return True

    def wait(self):
        self.p.wait()
        self.poll()

    def term(self):
        if self.running:
            if taskdb is not None:
                del taskdb[self.taskidx]
            self.running = False
            self.p.terminate()
            self.p.wait(2)
            self.p.kill()

    def __del__(self):
        self.term()


######################################################

if sys.argv[1] == "init":
    try:
        opts, args = getopt.getopt(sys.argv[2:], "f", ["nosetup"])
    except getopt.GetoptError as err:
        print(err)
        usage()

    dosetup = True
    force = False
    for o, a in opts:
        if o == "--nosetup":
            dosetup = False
        if o == "-f":
            force = True

    if os.path.exists("database"):
        if not force:
            print("found existing database/ directory.")
            exit(1)
        else:
            try:
                os.remove("database/db.sqlite3")
            except FileNotFoundError:
                pass
    else:
        print("creating database directory")
        os.mkdir("database")

    if dosetup and cfg.setup:
        print("running setup")
        if not os.path.exists("database/setup"):
            os.mkdir("database/setup")
        with open("database/setup.sh", "w") as f:
            for line in cfg.setup:
                print(line, file=f)
        task = Task("bash database/setup.sh")
        task.wait()

    with open("database/design.ys", "w") as f:
        for line in cfg.script:
            print(line, file=f)
        print("write_ilang database/design.il", file=f)

    task = Task("yosys -ql database/design.log database/design.ys")
    task.wait()

    with open("database/mutations.ys", "w") as f:
        print("read_ilang database/design.il", file=f)
        print(f"mutate -list {cfg.opt_size} -seed {cfg.opt_seed} -none{''.join(' -cfg %s %d' % (k, v) for k, v, in sorted(cfg.mutopts.items()))}{' -mode ' + cfg.opt_mode if cfg.opt_mode else ''} -o database/mutations.txt -s database/sources.txt{' ' + ' '.join(cfg.select) if len(cfg.select) else ''}", file=f)

    task = Task("yosys -ql database/mutations.log database/mutations.ys")
    task.wait()

    print("initializing database")

    db = sqlite3_connect()
    db.executescript("""
        CREATE TABLE mutations (
            mutation_id INTEGER PRIMARY KEY,
            mutation STRING
        );

        CREATE TABLE options (
            mutation_id INTEGER,
            opt_type STRING,
            opt_value STRING
        );

        CREATE TABLE sources (
            srctag STRING
        );

        CREATE TABLE results (
            mutation_id INTEGER,
            test STRING,
            result STRING
        );

        CREATE TABLE tags (
            mutation_id INTEGER,
            tag STRING
        );

        CREATE TABLE queue (
            mutation_id INTEGER,
            test STRING,
            running BOOL
        );

        CREATE TABLE files (
            filename STRING,
            data BLOB
        );
    """)

    with open("database/mutations.txt", "r") as f:
        for line in f:
            mid = db.execute("INSERT INTO mutations (mutation) VALUES (?)", [line.rstrip()]).lastrowid
            optarray = line.split()
            skip_next = False
            for i in range(len(optarray)-1):
                if skip_next:
                    skip_next = False
                elif optarray[i].startswith("-"):
                    db.execute("INSERT INTO options (mutation_id, opt_type, opt_value) VALUES (?, ?, ?)", [mid, optarray[i][1:], optarray[i+1]])
                    skip_next = True

    with open("database/sources.txt", "r") as f:
        for line in f:
            db.execute("INSERT INTO sources (srctag) VALUES (?)", [line.strip()])

    for db_filename, os_filename in cfg.files.items():
        with open(os_filename, "rb") as f:
            db.execute("INSERT INTO files (filename, data) VALUES (?, ?)", [db_filename, sqlite3.Binary(f.read())])

    db.commit()

    reset_status(db, True)

    exit(0)


if sys.argv[1] in ("reset", "status"):
    db = sqlite3_connect(chkexist=True)
    reset_status(db, sys.argv[1] == "reset")
    print_report(db)
    exit(0)

if sys.argv[1] == "purge":
    shutil.rmtree("tasks", ignore_errors=True)
    shutil.rmtree("database", ignore_errors=True)
    exit(0)

######################################################

if sys.argv[1] == "list":
    silent_sigpipe = True
    opt_details = False

    try:
        opts, args = getopt.getopt(sys.argv[2:], "", ["details"])
    except getopt.GetoptError as err:
        print(err)
        usage()

    for o, a in opts:
        if o == "--details":
            opt_details = True

    db = sqlite3_connect(chkexist=True)
    whitelist = None

    if len(args):
        whitelist = set()
        for a in args:
            if re.match("^[0-9]+$", a):
                whitelist.add(int(a))
            else:
                for mut, in db.execute("SELECT mutation_id FROM tags WHERE tag = ?", [a]):
                    whitelist.add(mut)

    if opt_details:
        print()

    for mid, mut in db.execute("SELECT mutation_id, mutation FROM mutations"):
        if whitelist is not None and mid not in whitelist:
            continue
        print("%d:" % mid, end="")
        found_tags = False
        for tag, in db.execute("SELECT tag FROM tags WHERE mutation_id = ?", [mid]):
            print(" %s" % tag, end="")
            found_tags = True
        if not found_tags:
            print(" no-tags", end="")
        for tst, rn in db.execute("SELECT test, running FROM queue WHERE mutation_id = ?", [mid]):
            if rn:
                print(" [%s]" % tst, end="")
            else:
                print(" (%s)" % tst, end="")
        print()

        if opt_details:
            print("  %s" % mut)
            for tst, res in db.execute("SELECT test, result FROM results WHERE mutation_id = ?", [mid]):
                print("  result from \"%s\": %s" % (tst, res))
            print()

    exit(0)


######################################################

def run_task(db, whitelist, tst=None, mut_list=None, verbose=False, keepdir=False):
    if tst is None or mut_list is None:
        assert tst is None
        assert mut_list is None
        db.execute("BEGIN EXCLUSIVE")

        # Find test for next task
        entry = db.execute("SELECT test, COUNT(*) as cnt FROM queue WHERE running = 0 AND " + whitelist + " GROUP BY test ORDER BY cnt DESC LIMIT 1").fetchone()
        if entry is None:
            db.commit()
            return False
        tst, _ = entry
        t = tst.split()[0]
        tst_args = tst.lstrip()[len(t)+1:]

        # Find mutations for next task
        mut_list = list([mut for mut, in db.execute("SELECT mutation_id FROM queue WHERE running = 0 AND test = ? AND " + whitelist + " ORDER BY mutation_id ASC LIMIT ?", [tst, cfg.tests[t].maxbatchsize])])
    else:
        t = tst.split()[0]
        tst_args = tst.lstrip()[len(t)+1:]

    # Mark tests running in DB (if we are killed after this, "mcy reset" is needed to re-create the queue entries)
    for mut in mut_list:
        db.execute("UPDATE queue SET running = 1 WHERE mutation_id = ? AND test = ?", [mut, tst])
        running.add((mut, tst))
    db.commit()

    task_id = str(uuid.uuid4())
    os.makedirs("tasks/%s" % task_id)

    infomsgs = list()
    infomsgs.append("task %s (%s)" % (task_id, tst))
    print("task %s (%s)" % (task_id, tst))

    with open("tasks/%s/input.txt" % task_id, "w") as f:
        for idx, mut in enumerate(mut_list):
            try:
                mut_str, = db.execute("SELECT mutation FROM mutations WHERE mutation_id = ?", [mut]).fetchone()
            except:
                print("Mutation number %s' not found in database." % mut)
                exit(1)
                pass
            infomsgs.append("  %d %d %s" % (idx+1, mut, mut_str))
            print("  %d %d %s" % (idx+1, mut, mut_str))
            print("%d %s" % (idx+1, mut_str), file=f)

    def callback():
        print("task %s (%s) finished." % (task_id, tst))
        checklist = set(mut_list)
        with open("tasks/%s/output.txt" % task_id, "r") as f:
            for line in f:
                line = line.split()
                if (len(line) != 2):
                    raise Exception('Invalid line format in file tasks/%s/output.txt' % task_id)
                idx = int(line[0])-1
                mut = mut_list[idx]
                if not mut in checklist:
                    raise Exception('Unknown mutation %s in file tasks/%s/output.txt' % (mut, task_id))
                res = line[1]
                if cfg.tests[t].expect is not None:
                    if not res in cfg.tests[t].expect:
                        raise Exception('Executing %s resulted with %s expecting value(s): %s' % (tst, res, ', '.join(cfg.tests[t].expect)))
                db.execute("DELETE FROM results WHERE mutation_id = ? AND test = ?", [mut, tst])
                db.execute("INSERT INTO results (mutation_id, test, result) VALUES (?, ?, ?)", [mut, tst, res])
                update_mutation(db, mut)
                running.remove((mut, tst))
                checklist.remove(mut)
                print("  %d %d %s %s" % (idx+1, mut, res, mut_str))

        if len(checklist) != 0:
            raise Exception('Empty mutation checklist')
        if not keepdir:
            shutil.rmtree("tasks/%s" % task_id)
            try:
                os.rmdir("tasks/")
            except OSError:
                pass
    script_path = root_path() + '/../share/mcy/scripts' # for install
    if (not os.path.exists(script_path)):
        script_path = root_path() + '/scripts' # for development
    command = "export TASK=%s PRJDIR=\"$PWD\" KEEPDIR=%d MUTATIONS=\"%s\" SCRIPTS=\"%s\"; cd tasks/$TASK; export TASKDIR=\"$PWD\"" % \
            (task_id, 1 if keepdir else 0, " ".join(["%d" % mut for mut in mut_list]), script_path)
    logfilename = None
    if not verbose:
        with open("tasks/%s/logfile.txt" % task_id, "w") as f:
            for msg in infomsgs: print(msg, file=f)
        command += "; exec >>logfile.txt"
        logfilename = "tasks/%s/logfile.txt" % task_id
    if (t not in cfg.tests):
        print("Test '%s' not found." % t)
        exit(1)
    command += "; %s %s" % (cfg.tests[t].run, tst_args)
    task = Task(command, callback, silent=(not verbose), logfilename=logfilename)

    return True

if sys.argv[1] == "run":
    opt_jobs = 1
    opt_reset = False

    try:
        opts, args = getopt.getopt(sys.argv[2:], "j:", ["reset"])
    except getopt.GetoptError as err:
        print(err)
        usage()

    for o, a in opts:
        if o == "-j":
            opt_jobs = int(a)
        elif o == "--reset":
            opt_reset = True

    db = sqlite3_connect(chkexist=True)
    whitelist = "1"

    if len(args):
        whitelist = "("
        for a in args:
            if whitelist != "(":
                whitelist += " OR "
            if re.match("^[0-9]+$", a):
                whitelist += "mutation_id = %d" % int(a)
            else:
                for mut, in db.execute("SELECT mutation_id FROM tags WHERE tag = ?", [a]):
                    whitelist += "mutation_id = %d" % mut
        whitelist += ")"

    if opt_reset:
        reset_status(db, True)

    while run_task(db, whitelist) or len(taskdb):
        wait_tasks(opt_jobs)

    wait_tasks(1)
    reset_status(db)
    print_report(db)
    exit(0)

if sys.argv[1] == "task":
    opt_verbose = False
    opt_keepdir = False

    try:
        opts, args = getopt.getopt(sys.argv[2:], "vk", [])
    except getopt.GetoptError as err:
        print(err)
        usage()

    for o, a in opts:
        if o == "-v":
            opt_verbose = True
        elif o == "-k":
            opt_keepdir = True
        else:
            assert False

    if len(args) < 2:
        usage()

    db = sqlite3_connect(chkexist=True)

    tst = args[0]
    mut_list = list()
    for a in args[1:]:
        if re.match("^[0-9]+$", a):
            if int(a) not in mut_list:
                mut_list.append(int(a))
        else:
            for mut, in db.execute("SELECT mutation_id FROM tags WHERE tag = ? ORDER BY mutation_id ASC", [a]):
                if mut not in mut_list:
                    mut_list.append(mut)

    if len(mut_list) == 0:
        raise Exception('Task not found')
    run_task(db, "1", tst, mut_list, verbose=opt_verbose, keepdir=opt_keepdir)
    wait_tasks(1)
    exit(0)


######################################################

if sys.argv[1] == "source":
    silent_sigpipe = True
    opt_encoding = "utf8"

    try:
        opts, args = getopt.getopt(sys.argv[2:], "e:", [])
    except getopt.GetoptError as err:
        print(err)
        usage()

    for o, a in opts:
        if o == "-e":
            opt_encoding = a
        else:
            assert False

    if len(args) not in [1, 2]:
        usage()

    db = sqlite3_connect(chkexist=True)

    if len(args) == 1:
        filename = args[0]
        filedata = db.execute("SELECT data FROM files WHERE filename = ?", [filename]).fetchone()
        if filedata is None:
            print("File data for '%s' not found in database." % filename)
            exit(1)
        filedata = str(filedata[0], opt_encoding)
    elif len(args) == 2:
        filename = args[0]
        with open(args[1], "rb") as f:
            filedata = str(f.read(), opt_encoding)
    else:
        assert False

    # Fix DOS-style and old Macintosh-style line endings
    filedata = filedata.replace("\r\n", "\n").replace("\r", "\n")

    covercache = dict()

    for src, in db.execute("SELECT DISTINCT srctag FROM sources WHERE srctag LIKE ?", [filename + ":%"]):
        src = src.replace(filename + ":","")
        if ("." in src):
            src = src[:src.index(".")]
        covercache[src] = types.SimpleNamespace(covered=0, uncovered=0, used=0)

    for src, covered, uncovered in db.execute("""
          SELECT opt_value,
                 COUNT(CASE WHEN tag =   'COVERED' THEN 1 END),
                 COUNT(CASE WHEN tag = 'UNCOVERED' THEN 1 END)
            FROM options
            JOIN tags ON (options.mutation_id = tags.mutation_id)
           WHERE opt_type = 'src'
             AND opt_value LIKE ?
        GROUP BY opt_value
    """, [filename + ":%"]):
        src = src.replace(filename + ":","")
        if ("." in src):
            src = src[:src.index(".")]
        covercache[src].covered += covered
        covercache[src].uncovered += uncovered
        covercache[src].used = 1

    for linenr, line in enumerate(filedata.rstrip("\n").split("\n")):
        src = "%d" % (linenr+1)

        if src in covercache:
            if covercache[src].used:
                if covercache[src].uncovered:
                    print("!%4d|\t" % -covercache[src].uncovered, end="")
                else:
                    print("%5d|\t" % covercache[src].covered, end="")
            else:
                print("    ?|\t", end="")
        else:
            print("     |\t", end="")

        print(line)

    exit(1)


######################################################

if sys.argv[1] == "lcov":
    silent_sigpipe = True
    opt_encoding = "utf8"

    try:
        opts, args = getopt.getopt(sys.argv[2:], "e:", [])
    except getopt.GetoptError as err:
        print(err)
        usage()

    if len(args) not in [1]:
        usage()

    db = sqlite3_connect(chkexist=True)

    if len(args) == 1:
        filename = args[0]
        filedata = db.execute("SELECT data FROM files WHERE filename = ?", [filename]).fetchone()
        if filedata is None:
            print("File data for '%s' not found in database." % filename)
            exit(1)
        filedata = str(filedata[0], opt_encoding)
    else:
        assert False

    # Fix DOS-style and old Macintosh-style line endings
    filedata = filedata.replace("\r\n", "\n").replace("\r", "\n")

    covercache = dict()

    for src, in db.execute("SELECT DISTINCT srctag FROM sources WHERE srctag LIKE ?", [filename + ":%"]):
        covercache[src] = types.SimpleNamespace(covered=0, uncovered=0)

    for src, covered, uncovered in db.execute("""
          SELECT opt_value,
                 COUNT(CASE WHEN tag =   'COVERED' THEN 1 END),
                 COUNT(CASE WHEN tag = 'UNCOVERED' THEN 1 END)
            FROM options
            JOIN tags ON (options.mutation_id = tags.mutation_id)
           WHERE opt_type = 'src'
             AND opt_value LIKE ?
        GROUP BY opt_value
    """, [filename + ":%"]):
        covercache[src].covered += covered
        covercache[src].uncovered += uncovered

    lines_total = 0
    lines_covered = 0
    print("TN:")
    print("SF:%s" % filename)
    for linenr, line in enumerate(filedata.rstrip("\n").split("\n")):
        src = "%s:%d" % (filename, linenr+1)

        if src in covercache:
            lines_total += 1
            if covercache[src].uncovered:
                print("DA:%d,%d" % (linenr+1, 0))
            else:
                print("DA:%d,%d" % (linenr+1, covercache[src].covered))
                if (covercache[src].covered !=0):
                    lines_covered += 1

    print("LF:%d" % lines_total)
    print("LH:%d" % lines_covered)
    print("end_of_record")
    exit(1)


######################################################

if sys.argv[1] == "dash":
    os.execvp("mcy-dash", ["mcy-dash"] + sys.argv[2:])
    exit(1)

if sys.argv[1] == "gui":
    os.execvp("mcy-gui", ["mcy-gui"] + sys.argv[2:])
    exit(1)

usage()
