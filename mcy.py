#!/usr/bin/env python3

import getopt, sys, os, re, time
import subprocess, sqlite3, uuid
from types import SimpleNamespace

taskidx = 0
taskdb = dict()

def exit(rc):
    for task in list(taskdb.values()):
        task.term()
    sys.exit(rc)

def usage():
    print()
    print("Usage:")
    print("  mcy init")
    print("  mcy reset")
    print("  mcy status")
    print("  mcy list [--details] [id..]")
    print("  mcy run [-jN] [--reset] [id..]")
    print("  mcy source")
    print("  mcy dash")
    print("  mcy gui")
    print()
    exit(1)

if len(sys.argv) < 2:
    usage()

if not os.path.exists("config.mcy"):
    print("config.mcy not found")
    exit(1)

cfg = SimpleNamespace()
cfg.opt_size = 20
cfg.opt_tags = None
cfg.script = list()
cfg.logic = list()
cfg.report = list()
cfg.tests = dict()

with open("config.mcy", "r") as f:
    section = None
    sectionarg = None
    linenr = 0
    for line in f:
        linenr += 1

        match = re.match(r"^\[(.*)\]\s*$", line)
        if match:
            entries = match.group(1).split()
            if len(entries) == 1 and entries[0] in ("options", "script", "logic", "report"):
                section, sectionarg = entries[0], None
                continue
            if len(entries) == 2 and entries[0] == "test":
                section, sectionarg = entries
                if sectionarg not in cfg.tests:
                    cfg.tests[sectionarg] = SimpleNamespace()
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
            if len(entries) > 1 and entries[0] == "tags":
                cfg.opt_tags = set(entries[1:])
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

        print("syntax error in line %d in config.mcy" % linenr)
        exit(1)


######################################################

def xorshift32(x):
    x = (x ^ x << 13) & 0xFFFFFFFF
    x = (x ^ x >> 17) & 0xFFFFFFFF
    x = (x ^ x <<  5) & 0xFFFFFFFF
    return x;

def update_mutation(db, mid):
    rng_state = xorshift32(xorshift32(mid + 12345))
    rng_state = xorshift32(xorshift32(rng_state))

    db.execute("DELETE FROM queue WHERE mutation_id = ?;", [mid])
    db.execute("DELETE FROM tags WHERE mutation_id = ?;", [mid])

    class ResultNotReadyException(BaseException):
        def __init__(self, tst):
            self.tst = tst

    def env_result(tst):
        t = tst.split()[0]
        for res, in db.execute("SELECT (result) FROM results WHERE mutation_id = ? AND test = ?;", [mid, tst]):
            if cfg.tests[t].expect is not None:
                assert res in cfg.tests[t].expect
            return res
        raise ResultNotReadyException(tst)

    def env_tag(tag):
        if cfg.opt_tags is not None:
            assert tag in cfg.opt_tags
        db.execute("INSERT INTO tags (mutation_id, tag) VALUES (?, ?);", [mid, tag])

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
        db.execute("INSERT INTO queue (mutation_id, test, running) VALUES (?, ?, 0);", [mid, ex.tst])

    db.commit()

def reset_status(db, do_reset=False):
    if do_reset:
        for mid, in db.execute("SELECT mutation_id FROM mutations;"):
            update_mutation(db, mid)

    for tst, res, cnt in db.execute("SELECT test, result, COUNT(*) FROM results GROUP BY test, result"):
        print("Database contains %d cached \"%s\" results for \"%s\"." % (cnt, res, tst))

    for tag, cnt in db.execute("SELECT tag, COUNT(*) FROM tags GROUP BY tag"):
        print("Tagged %d mutations as \"%s\"." % (cnt, tag))

    for tst, cnt, rn in db.execute("SELECT test, COUNT(*), SUM(running) FROM queue GROUP BY test"):
        if rn > 0:
            print("Queued %d tasks for test \"%s\", %d running." % (cnt, tst, rn))
        else:
            print("Queued %d tasks for test \"%s\"." % (cnt, tst))

def print_report(db):
    def env_tags(tag=None):
        if tag is None:
            cnt, = db.execute("select count(*) from (select 1 from tags group by mutation_id);").fetchone()
            return cnt

        invert = False
        if tag.startswith("!"):
            invert = True
            tag = tag[1:]

        cnt, = db.execute("SELECT count(*) FROM tags WHERE tag = ?", [tag]).fetchone()

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
    def __init__(self, command, callback=None, infn=None, outfn=None):
        global taskidx
        taskidx += 1
        self.taskidx = taskidx
        self.command = command
        self.sub_stdin = subprocess.DEVNULL if infn is None else open(infn, "r")
        self.sub_stdout = None if infn is None else open(outfn, "w")
        print("%s" % command)
        self.callback = callback
        self.p = subprocess.Popen(command, shell=True, stdin=self.sub_stdin, stdout=self.sub_stdout)
        taskdb[taskidx] = self

    def poll(self):
        rc = self.p.poll()
        if rc == None:
            return False
        if self.taskidx in taskdb:
            del taskdb[self.taskidx]
        if rc != 0:
            print("Command '%s' returned non-zero return code %d." % (self.command, rc))
            exit(1)
        if self.callback is not None:
            self.callback()
            self.callback = None
        return True

    def wait(self):
        self.p.wait()
        self.poll()

    def term(self):
        if self.taskidx in taskdb:
            del taskdb[self.taskidx]
            self.p.terminate()
            self.p.wait(2)
            self.p.kill()

    def __del__(self):
        self.term()


######################################################

if sys.argv[1] == "init":
    try:
        opts, args = getopt.getopt(sys.argv[2:], "", [])
    except getopt.GetoptError as err:
        print(err)
        usage()

    for o, a in opts:
        pass

    if os.path.exists("database"):
        print("found existing database/ directory.")
        exit(1)

    print("creating database directory")
    os.mkdir("database")

    with open("database/design.ys", "w") as f:
        for line in cfg.script:
            print(line, file=f)
        print("mutate -list %d -o database/mutations.txt" % cfg.opt_size, file=f)
        print("write_ilang database/design.il", file=f)

    task = Task("yosys -ql database/design.log database/design.ys")
    task.wait()

    print("initializing database")

    db = sqlite3.connect("database/db.sqlite3")
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
    """)

    with open("database/mutations.txt", "r") as f:
        for line in f:
            mid = db.execute("INSERT INTO mutations (mutation) VALUES (?);", [line.rstrip()]).lastrowid
            optarray = line.split()
            skip_next = False
            for i in range(len(optarray)-1):
                if skip_next:
                    skip_next = False
                elif optarray[i].startswith("-"):
                    db.execute("INSERT INTO options (mutation_id, opt_type, opt_value) VALUES (?, ?, ?);", [mid, optarray[i][1:], optarray[i+1]])
                    skip_next = True
    db.commit()

    reset_status(db, True)

    exit(0)


if sys.argv[1] in ("reset", "status"):
    db = sqlite3.connect("database/db.sqlite3")
    reset_status(db, sys.argv[1] == "reset")
    print_report(db)
    exit(0)


######################################################

if sys.argv[1] == "list":
    opt_details = False

    try:
        opts, args = getopt.getopt(sys.argv[2:], "", ["details"])
    except getopt.GetoptError as err:
        print(err)
        usage()

    for o, a in opts:
        if o == "--details":
            opt_details = True

    db = sqlite3.connect("database/db.sqlite3")
    whitelist = None

    if len(args):
        whitelist = set()
        for a in args:
            whitelist.add(int(a))

    if opt_details:
        print()

    for mid, mut in db.execute("SELECT mutation_id, mutation FROM mutations;"):
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

def run_task(db, whitelist):
    # Find test for next task
    entry = db.execute("SELECT test, count(*) as cnt FROM queue WHERE running = 0 AND " + whitelist + " GROUP BY test ORDER BY cnt DESC LIMIT 1").fetchone()
    if entry is None:
        return False
    tst, _ = entry
    t = tst.split()[0]
    tst_args = tst.lstrip()[len(t)+1:]

    # Find mutations for next task
    mut_list = list([mut for mut, in db.execute("SELECT mutation_id FROM queue WHERE running = 0 AND test = ? AND " + whitelist + " ORDER BY mutation_id ASC LIMIT ?", [tst, cfg.tests[t].maxbatchsize])])

    # Remove mutations from DB (if anything fails after this, "mcy reset" is needed to re-create the queue entries)
    for mut in mut_list:
        db.execute("UPDATE queue SET running = 1 WHERE mutation_id = ? AND test = ?", [mut, tst])
    db.commit()

    task_id = str(uuid.uuid4())
    print("task %s (%s)" % (task_id, tst))

    with open("database/task_%s.in" % task_id, "w") as f:
        for idx, mut in enumerate(mut_list):
            mut_str, = db.execute("SELECT mutation FROM mutations WHERE mutation_id = ?", [mut]).fetchone()
            print("  %d %d %s" % (idx+1, mut, mut_str))
            print("%d: %s" % (idx+1, mut_str), file=f)

    def callback():
        print("task %s finished." % task_id)
        with open("database/task_%s.out" % task_id, "r") as f:
            for line in f:
                line = line.split()
                assert len(line) == 2
                assert line[0].endswith(":")
                idx = int(line[0][:-1])-1
                mut = mut_list[idx]
                res = line[1]
                if cfg.tests[t].expect is not None:
                    assert res in cfg.tests[t].expect
                db.execute("INSERT INTO results (mutation_id, test, result) VALUES (?, ?, ?);", [mut, tst, res])
                print("  %d %d %s %s" % (idx+1, mut, res, mut_str))
        db.commit()
        os.remove("database/task_%s.in" % task_id)
        os.remove("database/task_%s.out" % task_id)
        for mut in mut_list:
            update_mutation(db, mut)

    command = "TASK=%s %s %s" % (task_id, cfg.tests[t].run, tst_args)
    task = Task(command, callback, "database/task_%s.in" % task_id, "database/task_%s.out" % task_id)

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

    db = sqlite3.connect("database/db.sqlite3")
    whitelist = "1"

    if len(args):
        whitelist = "("
        for a in args:
            if whitelist != "(":
                whitelist += " OR "
            whitelist += "mutation_id = %d" % int(a)
        whitelist += ")"

    if opt_reset:
        reset_status(db, True)

    while run_task(db, whitelist) or len(taskdb):
        wait_tasks(opt_jobs)

    wait_tasks(1)
    reset_status(db)
    print_report(db)
    exit(0)


######################################################

if sys.argv[1] == "source":
    print("'mcy source' is not implemented yet.")
    exit(1)


######################################################

if sys.argv[1] == "dash":
    print("'mcy dash' is not implemented yet.")
    exit(1)

if sys.argv[1] == "gui":
    print("'mcy gui' is not implemented yet.")
    exit(1)

usage()
