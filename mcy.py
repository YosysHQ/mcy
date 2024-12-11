#!/usr/bin/env python3

import sys, os, re, time, signal
import subprocess, sqlite3, uuid, shutil
import types
import click

TASKIDX = 0
TASKDB = dict()
RUNNING = set()
DBTRACE = False
SILENT_SIGPIPE = False

def log_warning(msg):
    """Log warning"""
    click.secho("==> WARNING : ", fg="yellow", nl=False, bold=True, err=True)
    click.secho(msg, fg="white", bold=True, err=True)

def log_error(msg):
    """Log error"""
    click.secho("==> ERROR : ", fg="red", nl=False, bold=True, err=True)
    click.secho(msg, fg="white", bold=True, err=True)
    exit_mcy(1)

def log_error_additional(msg, msg2):
    """Log error"""
    click.secho("==> ERROR : ", fg="red", nl=False, bold=True, err=True)
    click.secho(msg, fg="white", bold=True, err=True)
    click.secho(msg2, fg="white", bold=True, err=True)
    exit_mcy(1)

def log_info(msg):
    """Log info"""
    click.secho("==> ", fg="green", nl=False, bold=True, err=True)
    click.secho(msg, fg="white", bold=True, err=True)

def log_step(msg):
    """Log step"""
    click.secho("  -> ", fg="blue", nl=False, bold=True, err=True)
    click.secho(msg, fg="white", bold=True, err=True)

def log_sub_info(msg):
    """Log sub info"""
    click.secho("  ==> ", fg="green", nl=False, bold=True, err=True)
    click.secho(msg, fg="white", bold=True, err=True)

def log_sub_step(msg):
    """Log sub step"""
    click.secho("    -> ", fg="blue", nl=False, bold=True, err=True)
    click.secho(msg, fg="white", bold=True, err=True)

def root_path():
    """Return root path"""
    return os.path.abspath(os.path.dirname(getattr(sys.modules['__main__'], '__file__')))

def sqlite3_connect(log=True, chkexist=False):
    """Connect to sqlite3 database"""
    if chkexist and not os.path.exists("database/db.sqlite3"):
        log_error_additional("Project database not found.", "Run 'mcy init' to initialize the project.")
    if log:
        log_step("Connecting to database.")
    database = sqlite3.connect("database/db.sqlite3")
    if DBTRACE:
        if log:
            log_step("Enable database tracing.")
        database.set_trace_callback(print)
    return database

def exit_mcy(return_code):
    """Exit MCY and cleanup"""
    for task in list(TASKDB.values()):
        task.term()
    if len(RUNNING)>0:
        database = sqlite3_connect(log=False)
        log_step("Remove 'RUNNING' status for tasks from queue.")
        try:
            for mut, tst in RUNNING:
                database.execute("UPDATE queue SET running = 0 WHERE mutation_id = ? AND test = ?", [mut, tst])
            database.commit()
        except Exception:
            click.secho("==> ERROR : ", fg="red", nl=False, bold=True, err=True)
            click.secho("Error doing MCY cleanup.", fg="white", bold=True, err=True)
    sys.exit(return_code)

def force_shutdown(signum, frame):
    """Forced shutdown"""
    if (os.name != 'nt' and signum != signal.SIGPIPE) or not SILENT_SIGPIPE:
        click.secho("\nMCY ---- Keyboard interrupt or external termination signal ----", fg="red", nl=True, bold=True)
    exit_mcy(1)

def xorshift32(x):
    """Seed generator helper"""
    x = (x ^ x << 13) & 0xFFFFFFFF
    x = (x ^ x >> 17) & 0xFFFFFFFF
    x = (x ^ x <<  5) & 0xFFFFFFFF
    return x

def read_cfg():
    """Read configuration file"""
    log_step("Reading configuration file.")

    if not os.path.exists("config.mcy"):
        log_error("config.mcy not found")

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
                log_error(f"Syntax error in line {linenr} of config.mcy")

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

            log_error(f"Syntax error in line {linenr} of config.mcy")

    if cfg.opt_seed is None:
        cfg.opt_seed = int(100 * time.time())
        cfg.opt_seed = xorshift32(cfg.opt_seed)
        cfg.opt_seed = xorshift32(cfg.opt_seed)
        cfg.opt_seed = xorshift32(cfg.opt_seed)
        cfg.opt_seed = cfg.opt_seed % 1000000000

    return cfg

######################################################

def update_mutation(db, cfg, mid):
    """Update mutation"""
    rng_state = xorshift32(xorshift32(mid + cfg.opt_seed))
    rng_state = xorshift32(xorshift32(rng_state))

    db.execute("DELETE FROM queue WHERE mutation_id = ?", [mid])
    db.execute("DELETE FROM tags WHERE mutation_id = ?", [mid])

    class ResultNotReadyException(BaseException):
        """ResultNotReadyException"""
        def __init__(self, tst):
            """constructor"""
            BaseException.__init__(self)
            self.tst = tst

    def env_result(tst):
        t = tst.split()[0]
        for res, in db.execute("SELECT (result) FROM results WHERE mutation_id = ? AND test = ?", [mid, tst]):
            if cfg.tests[t].expect is not None:
                if not res in cfg.tests[t].expect:
                    log_error(f"Executing {tst} resulted with {res} expecting value(s): {', '.join(cfg.tests[t].expect)}")
            return res
        raise ResultNotReadyException(tst)

    def env_tag(tag):
        if cfg.opt_tags is not None:
            if not tag in cfg.opt_tags:
                log_error(f"Provided tag {tag} not one of expected: {', '.join(cfg.opt_tags)}")
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

def reset_status(db, cfg, do_reset=False):
    """Reset status"""
    if do_reset:
        nmutations, = db.execute("SELECT COUNT(*) FROM mutations").fetchone()
        if nmutations < cfg.opt_size:
            log_step(f"Adding {(cfg.opt_size - nmutations)} mutations to database.")

            log_step("Creating additional mutations script file.")
            with open("database/mutations2.ys", "w") as f:
                print("read_rtlil database/design.il", file=f)
                print(f"mutate -list {cfg.opt_size} -seed {cfg.opt_seed} -none{''.join(' -cfg %s %d' % (k, v) for k, v, in sorted(cfg.mutopts.items()))}{' -mode ' + cfg.opt_mode if cfg.opt_mode else ''} -o database/mutations2.txt -s database/sources.txt{' ' + ' '.join(cfg.select) if len(cfg.select) else ''}", file=f)

            log_step("Creating additional mutations.")
            task = Task("yosys -ql database/mutations2.log database/mutations2.ys")
            task.wait()

            log_step("Inserting additional mutations in database.")
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

        log_step("Remove 'tasks' subdirectory.")
        shutil.rmtree("tasks", ignore_errors=True)

        log_step("Update mutations.")
        for mid, in db.execute("SELECT mutation_id FROM mutations"):
            update_mutation(db, cfg, mid)

    log_step("Getting database statistics.")
    cnt, = db.execute("SELECT COUNT(*) FROM results").fetchone()
    print(f"Database contains {cnt} cached results.")

    for tst, res, cnt in db.execute("SELECT test, result, COUNT(*) FROM results GROUP BY test, result"):
        print(f"Database contains {cnt} cached \"{res}\" results for \"{tst}\".")

    for tag, cnt in db.execute("SELECT tag, COUNT(*) FROM tags GROUP BY tag"):
        print(f"Tagged {cnt} mutations as \"{tag}\".")

    for tst, cnt, rn in db.execute("SELECT test, COUNT(*), SUM(running) FROM queue GROUP BY test"):
        if rn > 0:
            print(f"Queued {cnt} \"{tst}\" tests, {rn} running.")
        else:
            print(f"Queued {cnt} \"{tst}\" tests.")

def print_report(db, cfg):
    """Print report"""
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

    log_step("Print report")
    gdict = globals().copy()
    gdict["tags"] = env_tags

    code = "def __report__():\n  " + "\n  ".join(cfg.report) + "\n__report__()\n"
    exec(code, gdict)

######################################################

def wait_tasks(num):
    """Wait for tasks"""
    global TASKDB
    for task in list(TASKDB.values()):
        task.poll()

    while len(TASKDB) >= num:
        time.sleep(0.5)  # Ugh!
        for task in list(TASKDB.values()):
            task.poll()

class Task:
    """Task class"""
    def __init__(self, command, callback=None, silent=False, logfilename=None):
        """constructor"""
        global TASKIDX
        TASKIDX += 1
        self.taskidx = TASKIDX
        self.command = command
        if not silent:
            print(command)
        self.callback = callback
        self.logfilename = logfilename
        self.p = subprocess.Popen(command, shell=True, stdin=subprocess.DEVNULL)
        TASKDB[TASKIDX] = self
        self.running = True

    def poll(self):
        """poll"""
        if not self.running:
            return True
        return_code = self.p.poll()
        if return_code is None:
            return False
        self.running = False
        if self.taskidx in TASKDB:
            del TASKDB[self.taskidx]
        if return_code != 0:
            log_error_additional(f"Command '{self.command}' returned non-zero return code {return_code}.",
                f"See '{self.logfilename}' for details." if self.logfilename is not None else ""
            )
        if self.callback is not None:
            self.callback()
            self.callback = None
        return True

    def wait(self):
        """wait"""
        self.p.wait()
        self.poll()

    def term(self):
        """term"""
        if self.running:
            if TASKDB is not None:
                del TASKDB[self.taskidx]
            self.running = False
            self.p.terminate()
            self.p.wait(2)
            self.p.kill()

    def __del__(self):
        """destructor"""
        self.term()

######################################################

@click.group(context_settings=dict(help_option_names=["-h", "--help"]), invoke_without_command=True)
@click.pass_context
def cli(ctx):
    """MCY - Mutation Cover with Yosys"""
    ctx.ensure_object(dict)
    if ctx.invoked_subcommand is None:
        click.secho(ctx.get_help())

@cli.command(name='init')
@click.option('-f', '--force', help='Force database initialization.', is_flag=True)
@click.option('--nosetup', help='Do not setup project.', is_flag=True)
@click.option('--trace', help='Trace database operations.', is_flag=True)
def init_command(force, nosetup, trace):
    """Initialize database"""
    global DBTRACE
    DBTRACE = trace
    log_info("Initialize database")

    cfg = read_cfg()

    log_step("Checking database existance.")
    if os.path.exists("database"):
        if not force:
            log_error("Found existing database directory.")
        else:
            try:
                os.remove("database/db.sqlite3")
            except FileNotFoundError:
                pass
    else:
        log_step("Creating database directory.")
        os.mkdir("database")

    if not nosetup and cfg.setup:
        log_step("Running setup.")
        if not os.path.exists("database/setup"):
            os.mkdir("database/setup")
        with open("database/setup.sh", "w") as f:
            for line in cfg.setup:
                print(line, file=f)
        task = Task("bash database/setup.sh")
        task.wait()

    log_step("Creating design script file.")
    with open("database/design.ys", "w") as f:
        for line in cfg.script:
            print(line, file=f)
        print("write_rtlil database/design.il", file=f)

    log_step("Creating design RTL.")
    task = Task("yosys -ql database/design.log database/design.ys")
    task.wait()

    log_step("Creating mutations script file.")
    with open("database/mutations.ys", "w") as f:
        print("read_rtlil database/design.il", file=f)
        print(f"mutate -list {cfg.opt_size} -seed {cfg.opt_seed} -none{''.join(' -cfg %s %d' % (k, v) for k, v, in sorted(cfg.mutopts.items()))}{' -mode ' + cfg.opt_mode if cfg.opt_mode else ''} -o database/mutations.txt -s database/sources.txt{' ' + ' '.join(cfg.select) if len(cfg.select) else ''}", file=f)

    log_step("Creating mutations.")
    task = Task("yosys -ql database/mutations.log database/mutations.ys")
    task.wait()

    log_step("Initializing database.")

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

    log_step("Importing mutations.")
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

    log_step("Importing design sources.")
    with open("database/sources.txt", "r") as f:
        for line in f:
            db.execute("INSERT INTO sources (srctag) VALUES (?)", [line.strip()])

    for db_filename, os_filename in cfg.files.items():
        with open(os_filename, "rb") as f:
            db.execute("INSERT INTO files (filename, data) VALUES (?, ?)", [db_filename, sqlite3.Binary(f.read())])

    db.commit()

    log_step("Reseting database statistics.")
    reset_status(db, cfg, True)

    exit_mcy(0)

@cli.command(name='reset')
@click.option('--trace', help='Trace database operations.', is_flag=True)
def reset_command(trace):
    """Reset database"""
    global DBTRACE
    DBTRACE = trace
    log_info("Reset database")

    cfg = read_cfg()
    db = sqlite3_connect(chkexist=True)
    reset_status(db, cfg, True)
    print_report(db, cfg)
    exit_mcy(0)

@cli.command(name='status')
@click.option('--trace', help='Trace database operations.', is_flag=True)
def status_command(trace):
    """Database status"""
    global DBTRACE
    DBTRACE = trace
    log_info("Database status")

    cfg = read_cfg()
    db = sqlite3_connect(chkexist=True)
    reset_status(db, cfg, False)
    print_report(db, cfg)
    exit_mcy(0)

@cli.command(name='purge')
def purge_command():
    """Purge database"""
    log_info("Purge database")

    read_cfg()
    log_step("Remove 'tasks' subdirectory.")
    shutil.rmtree("tasks", ignore_errors=True)
    log_step("Remove 'database' subdirectory.")
    shutil.rmtree("database", ignore_errors=True)
    exit_mcy(0)

@cli.command(name='list', short_help='List mutations')
@click.argument('filter', nargs=-1)
@click.option('-o', '--output', type=click.File('w'), default='-')
@click.option('--details', help='Show details.', is_flag=True)
@click.option('--trace', help='Trace database operations.', is_flag=True)
def list_command(filter, output, details, trace):
    """List mutations\b

       List all mutations or add FILTER by listing mutations or tag names"""
    global SILENT_SIGPIPE, DBTRACE
    SILENT_SIGPIPE = True
    DBTRACE = trace
    log_info("List mutations")

    read_cfg()

    db = sqlite3_connect(chkexist=True)
    whitelist = set()

    if len(filter) > 0:
        for arg in filter:
            if re.match("^[0-9]+$", arg):
                whitelist.add(int(arg))
            else:
                for mut, in db.execute("SELECT mutation_id FROM tags WHERE tag = ?", [arg]):
                    whitelist.add(mut)

    log_step("Write mutation list")
    if details:
        print(file=output)

    for mid, mut in db.execute("SELECT mutation_id, mutation FROM mutations"):
        if len(whitelist) > 0 and mid not in whitelist:
            continue
        print(f"{mid}:", end="", file=output)
        found_tags = False
        for tag, in db.execute("SELECT tag FROM tags WHERE mutation_id = ?", [mid]):
            print(f" {tag}", end="", file=output)
            found_tags = True
        if not found_tags:
            print(" no-tags", end="", file=output)
        for tst, rn in db.execute("SELECT test, running FROM queue WHERE mutation_id = ?", [mid]):
            if rn:
                print(f" [{tst}]", end="", file=output)
            else:
                print(f" ({tst})", end="", file=output)
        print(file=output)

        if details:
            print(f"  {mut}", file=output)
            for tst, res in db.execute("SELECT test, result FROM results WHERE mutation_id = ?", [mid]):
                print(f"  result from \"{tst}\": {res}", file=output)
            print(file=output)

    exit_mcy(0)

def run_task(db, cfg, whitelist, tst=None, mut_list=None, verbose=False, details=False, keepdir=False):
    """Run task"""
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
    task_id = str(uuid.uuid4())
    log_sub_info(f"Running task {task_id} ({tst})")

    if verbose:
        log_sub_step("Set status to 'RUNNING' for task.")
    for mut in mut_list:
        db.execute("UPDATE queue SET running = 1 WHERE mutation_id = ? AND test = ?", [mut, tst])
        RUNNING.add((mut, tst))
    db.commit()

    if verbose:
        log_sub_step(f"Make 'tasks/{task_id}' subdirectory.")
    os.makedirs("tasks/%s" % task_id)

    infomsgs = list()
    infomsgs.append("task %s (%s)" % (task_id, tst))
    if verbose:
        log_sub_step(f"Task {task_id} ({tst}) started.")

    with open("tasks/%s/input.txt" % task_id, "w") as f:
        for idx, mut in enumerate(mut_list):
            try:
                mut_str, = db.execute("SELECT mutation FROM mutations WHERE mutation_id = ?", [mut]).fetchone()
            except Exception:
                log_error(f"Mutation number '{mut}' not found in database.")
            infomsgs.append("  %d %d %s" % (idx+1, mut, mut_str))
            print(f" {(idx+1)} {mut} {mut_str}")
            print(f"{(idx+1)} {mut_str}", file=f)

    def callback():
        log_sub_info(f"Finishing task {task_id} ({tst})")
        checklist = set(mut_list)
        if verbose:
            log_sub_step(f"Results:")
        with open("tasks/%s/output.txt" % task_id, "r") as f:
            for line in f:
                line = line.split()
                if (len(line) != 2):                   
                    log_error(f"Invalid line format in file tasks/{task_id}/output.txt")

                idx = int(line[0])-1
                mut = mut_list[idx]
                if not mut in checklist:
                    log_error(f"Unknown mutation {mut} in file tasks/{task_id}/output.txt")
                res = line[1]
                if cfg.tests[t].expect is not None:
                    if not res in cfg.tests[t].expect:
                        log_error(f"Executing {tst} resulted with {res} expecting value(s): {', '.join(cfg.tests[t].expect)}")
                db.execute("DELETE FROM results WHERE mutation_id = ? AND test = ?", [mut, tst])
                db.execute("INSERT INTO results (mutation_id, test, result) VALUES (?, ?, ?)", [mut, tst, res])
                update_mutation(db, cfg, mut)
                RUNNING.remove((mut, tst))
                checklist.remove(mut)
                print(f"  {idx+1} {mut} {res} {mut_str}")

        if len(checklist) != 0:
            log_error("Empty mutation checklist.")
        if not keepdir:
            if verbose:
                log_sub_step(f"Remove 'tasks/{task_id}' subdirectory.")
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
    if not details:
        with open("tasks/%s/logfile.txt" % task_id, "w") as f:
            for msg in infomsgs:
                print(msg, file=f)
        command += "; exec >>logfile.txt"
        logfilename = "tasks/%s/logfile.txt" % task_id
    if (t not in cfg.tests):
        log_error(f"Test '{t}' not found.")
    command += f"; {cfg.tests[t].run} {tst_args}"
    task = Task(command, callback, silent=(not details), logfilename=logfilename)
    return True



@cli.command(name='run', short_help='Run all tasks')
@click.argument('filter', nargs=-1)
@click.option('-j', '--nproc', default=os.cpu_count(), show_default=True, help='Number of build process.')
@click.option('-v', '--verbose', help='Verbose output.', is_flag=True)
@click.option('--reset', help='Reset database before run.', is_flag=True)
@click.option('--trace', help='Trace database operations.', is_flag=True)
def run_command(filter, nproc, verbose, reset, trace):
    """Run all tasks\b

       Run all tasks from queue.
       Optionally FILTER by list of mutations or tag names can be provided."""
    global DBTRACE
    DBTRACE = trace
    log_info("Run all tasks from queue")

    cfg = read_cfg()
    db = sqlite3_connect(chkexist=True)
    whitelist = "1"

    if len(filter) > 0:
        whitelist = "("
        for arg in filter:
            if whitelist != "(":
                whitelist += " OR "
            if re.match("^[0-9]+$", arg):
                whitelist += f"mutation_id = {int(arg)}"
            else:
                for mut, in db.execute("SELECT mutation_id FROM tags WHERE tag = ?", [arg]):
                    whitelist += f"mutation_id = {mut}"
        whitelist += ")"

    if reset:
        reset_status(db, True)

    while run_task(db, cfg, whitelist, verbose = verbose) or len(TASKDB):
        wait_tasks(nproc)

    wait_tasks(1)
    log_step("Finished running all tasks.")
    reset_status(db, cfg)
    print_report(db, cfg)
    exit_mcy(0)

@cli.command(name='task', short_help='Run task')
@click.argument('test', nargs=1)
@click.argument('filter', nargs=-1)
@click.option('-v', '--verbose', help='Verbose output.', is_flag=True)
@click.option('-k', '--keepdir', help='Keep output directory.', is_flag=True)
@click.option('--trace', help='Trace database operations.', is_flag=True)
def task_command(test, filter, verbose, keepdir, trace):
    """Run task\b

       Run all tasks on queue with specific TEST.
       Optionally FILTER by list of mutations or tag names can be provided."""

    global DBTRACE
    DBTRACE = trace
    log_info("Run tasks from queue")

    cfg = read_cfg()
    db = sqlite3_connect(chkexist=True)
    mut_list = list()
    for arg in filter:
        if re.match("^[0-9]+$", arg):
            if int(arg) not in mut_list:
                mut_list.append(int(arg))
        else:
            for mut, in db.execute("SELECT mutation_id FROM tags WHERE tag = ? ORDER BY mutation_id ASC", [arg]):
                if mut not in mut_list:
                    mut_list.append(mut)

    if len(mut_list) == 0:
        log_error("Task not found.")

    run_task(db, cfg, "1", test, mut_list, details = verbose, keepdir = keepdir)
    wait_tasks(1)
    log_step("Finished running task.")
    exit_mcy(0)

def filename_help(ctx, filename):
    """Display additional help and list files"""
    if filename is None:
        click.echo(ctx.get_help()+'\n')
        if os.path.exists("database/db.sqlite3"):
            db = sqlite3_connect(log=False, chkexist=True)
            click.echo('Filenames in database:')
            for src, in db.execute("SELECT DISTINCT filename FROM files ORDER BY filename"):
                click.echo(f'  {src }')
            click.echo('')
        click.echo("Error: Missing argument 'FILENAME'.")
        exit_mcy(1)

@cli.command(name='source', short_help='Retrieve source info')
@click.argument('filename', nargs=1, metavar='FILENAME', required = False)
@click.argument('filepath', nargs=1, required = False)
@click.option('-o', '--output', type=click.File('w'), default='-')
@click.option('-e', '--encoding', default="utf8", show_default=True, help='Source code encoding.')
@click.option('--trace', help='Trace database operations.', is_flag=True)
@click.pass_context
def source_command(ctx, filename, filepath, output, encoding, trace):
    """Retrieve source info

       Retrieves source information for FILENAME from database.
       Optionaly load file from local FILEPATH if provided."""
    global DBTRACE
    DBTRACE = trace
    filename_help(ctx, filename)
    log_info("Retrieving source info")

    read_cfg()
    db = sqlite3_connect(chkexist=True)

    if filepath is None:
        filedata = db.execute("SELECT data FROM files WHERE filename = ?", [filename]).fetchone()
        if filedata is None:
            log_error(f"File data for '{filename}' not found in database.")
        filedata = str(filedata[0], encoding)
    else:
        with open(filepath, "rb") as f:
            filedata = str(f.read(), encoding)

    # Fix DOS-style and old Macintosh-style line endings
    filedata = filedata.replace("\r\n", "\n").replace("\r", "\n")

    log_step("Extract coverage info.")
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

    log_step("Display source file with info.")
    for linenr, line in enumerate(filedata.rstrip("\n").split("\n")):
        src = f"{linenr+1}"

        if src in covercache:
            if covercache[src].used:
                if covercache[src].uncovered:
                    print("!%4d|\t" % -covercache[src].uncovered, end="", file=output)
                else:
                    print("%5d|\t" % covercache[src].covered, end="", file=output)
            else:
                print("    ?|\t", end="", file=output)
        else:
            print("     |\t", end="", file=output)

        print(line, file=output)

    exit_mcy(0)

@cli.command(name='lcov', short_help='Retrieve coverage info')
@click.argument('filename', nargs=1, metavar='FILENAME', required = False)
@click.option('-o', '--output', type=click.File('w'), default='-')
@click.option('-e', '--encoding', default="utf8", show_default=True, help='Source code encoding.')
@click.option('--trace', help='Trace database operations.', is_flag=True)
@click.pass_context
def lcov_command(ctx, filename, output, encoding, trace):
    """Retrieve coverage info

       Displays coverage info for FILENAME in lcov file format"""
    global SILENT_SIGPIPE, DBTRACE
    SILENT_SIGPIPE = True
    DBTRACE = trace
    filename_help(ctx, filename)
    log_info("Retrieving coverage info")

    read_cfg()

    db = sqlite3_connect(chkexist=True)

    filedata = db.execute("SELECT data FROM files WHERE filename = ?", [filename]).fetchone()
    if filedata is None:
        log_error(f"File data for '{filename}' not found in database.")
    filedata = str(filedata[0], encoding)

    # Fix DOS-style and old Macintosh-style line endings
    filedata = filedata.replace("\r\n", "\n").replace("\r", "\n")

    covercache = dict()
    log_step("Extract coverage info.")
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
    log_step("Write coverage info.")
    print("TN:", file=output)
    print(f"SF:{filename}", file=output)
    for linenr, _ in enumerate(filedata.rstrip("\n").split("\n")):
        src = f"{filename}:{linenr+1}"

        if src in covercache:
            lines_total += 1
            if covercache[src].uncovered:
                print(f"DA:{linenr+1},0" % (linenr+1, 0), file=output)
            else:
                print(f"DA:{linenr+1},{covercache[src].covered}", file=output)
                if (covercache[src].covered !=0):
                    lines_covered += 1

    print(f"LF:{lines_total}", file=output)
    print(f"LH:{lines_covered}", file=output)
    print("end_of_record", file=output)
    exit_mcy(0)

@cli.command(name='dash', context_settings={"ignore_unknown_options": True})
@click.argument('args', nargs=-1)
def dash_command(args):
    """Execute dashboard"""
    try:
        os.execvp("mcy-dash", ["mcy-dash"] + list(args))
    except Exception:
        log_error("Error starting mcy-dash")

@cli.command(name='gui', context_settings={"ignore_unknown_options": True})
@click.argument('args', nargs=-1)
def gui_command(args):
    """Execute GUI"""
    try:
        os.execvp("mcy-gui", ["mcy-gui"] + list(args))
    except Exception:
        log_error("Error starting mcy-gui")

if __name__ == '__main__':
    if os.name == "posix":
        signal.signal(signal.SIGHUP, force_shutdown)
        signal.signal(signal.SIGPIPE, force_shutdown)
    signal.signal(signal.SIGINT, force_shutdown)
    signal.signal(signal.SIGTERM, force_shutdown)

    cli(None)
