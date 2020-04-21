import os
from threading import Thread
from applic import app , db
from flask_script import Manager, prompt_bool

manager = Manager(app)

@manager.command
def initdb():
    db.create_all()
    print ('Initialized the database')


@manager.command
def dropdb():
    if prompt_bool(
        "Are you sure you want to lose all your data"):
        db.drop_all()
        print ('Dropped the database')


@manager.command
def run():

    app.secret_key = os.urandom(12)
    #app.run(debug=True)
    app.run(debug = True, threaded = True)

if __name__ == '__main__':
    manager.run()

