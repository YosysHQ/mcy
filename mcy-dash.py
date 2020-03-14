#!/usr/bin/env python3

from flask import Flask, send_from_directory, render_template, send_file, request, redirect
import sqlite3, signal, os, sys, subprocess, time
import types

silent_sigpipe = False

def root_path():
    fn = getattr(sys.modules['__main__'], '__file__')
    root_path = os.path.abspath(os.path.dirname(fn))
    return root_path

path = root_path() + '/../share/mcy/dash' # for install
if (not os.path.exists(path)):
    path = root_path() + '/dash' # for development

app = Flask(__name__, root_path=path, static_url_path='')

def sqlite3_connect():
    db = sqlite3.connect("database/db.sqlite3")
    return db

def force_shutdown(signum, frame):
    if (os.name != 'nt' and signum != signal.SIGPIPE) or not silent_sigpipe:
        print("MCY ---- Keyboard interrupt or external termination signal ----", file=sys.stderr, flush=True)
    sys.exit(1)

if os.name == "posix":
    signal.signal(signal.SIGHUP, force_shutdown)
    signal.signal(signal.SIGPIPE, force_shutdown)
signal.signal(signal.SIGINT, force_shutdown)
signal.signal(signal.SIGTERM, force_shutdown)

if not os.path.exists("config.mcy"):
    print("config.mcy not found")
    sys.exit(1)


@app.route("/", methods=['GET', 'POST'])
@app.route("/index.html", methods=['GET', 'POST'])
def home():
    cnt_mutations = None
    cnt_queue = None
    cnt_results = None
    cnt_sources = None
    results = None
    tags = None
    queue = None
    running = None
    errorCode = 0
    if request.method == 'POST':
        action = request.form['action']
        if (action and action=='initialize'):
            subprocess.call(["mcy" ,'init'])
            return redirect('index.html')
        if (action and action=='run'):            
            subprocess.Popen(["mcy" ,'run','-j2'], close_fds=True)
            cnt_queue = 0
            while cnt_queue == 0:
                try:
                    time.sleep(2)
                    db = sqlite3_connect()
                    cnt_queue = db.execute('SELECT COUNT(*) FROM queue WHERE running=1').fetchone()[0]
                    db.close()                                        
                except:
                    print("Problem accessing database...")
            return redirect('index.html')
    try:
        db = sqlite3_connect()
        cnt_mutations = db.execute('SELECT COUNT(*) FROM mutations').fetchone()[0]
        cnt_queue = db.execute('SELECT COUNT(*) FROM queue').fetchone()[0]
        cnt_results = db.execute('SELECT COUNT(*) FROM results').fetchone()[0]
        cnt_sources = db.execute('SELECT COUNT(*) FROM sources').fetchone()[0]
        results = db.execute('SELECT test, result,COUNT(*),ROUND(count(*) * 100.00 /(SELECT count(*) FROM results),2)  FROM results GROUP BY test, result').fetchall()
        tags = db.execute('SELECT tag,count(*),ROUND(count(*) * 100.00 /(SELECT count(*) FROM tags),2) FROM tags GROUP BY tag').fetchall()
        queue = db.execute('SELECT test,CASE running WHEN 0 THEN \'PENDING\' ELSE \'RUNNING\' END,count(*) FROM queue GROUP BY test,running ORDER BY running DESC,test ASC').fetchall()
        running = db.execute('SELECT count(*) FROM queue WHERE running=1').fetchone()[0]
        db.close()
    except:
        if (not os.path.exists("database/db.sqlite3")):
            errorCode = 1
        else:
            errorCode = 2
    return render_template('index.html', selected='index', cnt_mutations=cnt_mutations, cnt_queue=cnt_queue, 
                           cnt_results=cnt_results, cnt_sources=cnt_sources, results=results, tags=tags, 
                           running=running, queue=queue, errorCode=errorCode)

@app.route("/mutations.html")
def mutations():
    mutations = None
    tags = None
    errorCode = 0
    try:
        db = sqlite3_connect()
        tags = db.execute('SELECT DISTINCT tag FROM tags').fetchall()
        sql = 'SELECT m.mutation_id, m.mutation'
        for tag in tags:
            sql = sql + ',(select count(*) from tags t where t.mutation_id=m.mutation_id and t.tag==\''+ tag[0] + '\') '
        sql += ' FROM mutations m'
        mutations = db.execute(sql).fetchall()
        db.close()
    except:
        if (not os.path.exists("database/db.sqlite3")):
            errorCode = 1
        else:
            errorCode = 2
    return render_template('mutations.html', selected='mutations', mutations=mutations, tags=tags, errorCode=errorCode)

@app.route("/source.html", methods=['GET', 'POST'])
def source():
    errorCode = 0
    filedata = None
    num = 0
    filename = ""
    files = None
    covercache = dict()
    if request.method == 'POST':
        filename = request.form['filename']
    try:
        db = sqlite3_connect()
        files = db.execute('SELECT DISTINCT SUBSTR(srctag,0,INSTR(srctag,\':\')) FROM sources ORDER BY SUBSTR(srctag,0,INSTR(srctag,\':\'))').fetchall()
        if filename=="":
            filename = files[0][0]
        sql = 'SELECT data FROM files WHERE filename = "%s"' % filename        
        fd = db.execute(sql).fetchall()
        if (len(fd)==0):
            if (len(sys.argv) > 1):
                try:
                    with open(os.path.join(sys.argv[1],filename) , 'r') as myfile:
                        filedata = myfile.read()
                except:
                    filedata = ""                        
                    errorCode = 3
            else:
                filedata = ""
        else:
            filedata = fd[0][0].decode('unicode_escape')
        # Fix DOS-style and old Macintosh-style line endings
        filedata = filedata.replace("\r\n", "\n").replace("\r", "\n")
        num = len(filedata.split('\n'))        
       
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
        db.close()
    except:
        if (not os.path.exists("database/db.sqlite3")):
            errorCode = 1
        else:
            errorCode = 2
    return render_template('source.html', selected='source', files=files, source=filedata, filename=filename, num=num, covercache=covercache, errorCode=errorCode)

@app.route('/js/<path:path>')
def send_js(path):
    return send_from_directory('static/js', path)

@app.route('/css/<path:path>')
def send_css(path):
    return send_from_directory('static/css', path)

@app.route('/db.sqlite3')
def download_db():
    try:
        return send_file(os.path.join(os.getcwd(),'database','db.sqlite3'), attachment_filename='db.sqlite3')
    except Exception as e:
        return str(e)

if __name__ == "__main__":
    app.run(debug=True)
