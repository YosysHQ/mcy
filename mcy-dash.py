#!/usr/bin/env python3

from flask import Flask, send_from_directory, render_template, send_file
import sqlite3, signal, os, sys

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
    if signum != signal.SIGPIPE or not silent_sigpipe:
        print("MCY ---- Keyboard interrupt or external termination signal ----", file=sys.stderr, flush=True)
    exit(1)

if os.name == "posix":
    signal.signal(signal.SIGHUP, force_shutdown)
signal.signal(signal.SIGINT, force_shutdown)
signal.signal(signal.SIGTERM, force_shutdown)
signal.signal(signal.SIGPIPE, force_shutdown)

if not os.path.exists("config.mcy"):
    print("config.mcy not found")
    exit(1)


@app.route("/")
@app.route("/index.html")
def home():
    mutations = None
    queue = None
    results = None
    sources = None
    error = ''
    try:
        db = sqlite3_connect()
        mutations = db.execute('select count(*) from mutations').fetchone()[0]
        queue = db.execute('select count(*) from queue').fetchone()[0]
        results = db.execute('select count(*) from results').fetchone()[0]
        sources = db.execute('select count(*) from sources').fetchone()[0]
        db.close()
    except:
        error ='Error accessing database'
    return render_template('index.html', selected='index', mutations=mutations, queue=queue, results=results, sources=sources, error=error)

@app.route("/mutations.html")
def mutations():
    mutations = None
    error = ''
    try:
        db = sqlite3_connect()
        db.row_factory = sqlite3.Row
        cur = db.cursor()
        cur.execute('select * from mutations')
        mutations = cur.fetchall()
        db.close()
    except:
        error ='Error accessing database'
    return render_template('mutations.html', selected='mutations', mutations=mutations, error=error)

@app.route("/settings.html")
def settings():
    return render_template('settings.html', selected='settings' )

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
