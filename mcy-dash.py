#!/usr/bin/env python3

from flask import Flask, send_from_directory, render_template, send_file
import sqlite3, signal, os, sys

def root_path():
    # Infer the root path from the run file in the project root (e.g. manage.py)
    fn = getattr(sys.modules['__main__'], '__file__')
    root_path = os.path.abspath(os.path.dirname(fn))
    return root_path

app = Flask(__name__, root_path=root_path() + '/../share/mcy/dash', static_url_path='')

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
    db = sqlite3_connect()
    mutations = db.execute('select count(*) from mutations').fetchone()[0]
    queue = db.execute('select count(*) from queue').fetchone()[0]
    results = db.execute('select count(*) from results').fetchone()[0]
    sources = db.execute('select count(*) from sources').fetchone()[0]
    db.close()
    return render_template('index.html', selected='index', mutations=mutations, queue=queue, results=results, sources=sources)

@app.route("/mutations.html")
def mutations():
    db = sqlite3_connect()
    db.row_factory = sqlite3.Row
    cur = db.cursor()
    cur.execute('select * from mutations')
    mutations = cur.fetchall()
    db.close()
    return render_template('mutations.html', selected='mutations', mutations=mutations)

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
