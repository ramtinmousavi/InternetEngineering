import os
import time
from flask import Flask, request, jsonify
from flask_sqlalchemy import SQLAlchemy
from flask_marshmallow import Marshmallow
from flask_cors import  cross_origin
from datetime import datetime

#create flask application and config database
app = Flask(__name__)
#relative path. modify if failed
dir_path = os.path.dirname(os.path.dirname(os.path.abspath(__file__))).replace ("\\" , '/').split(':')[1]
#EXAMPLE:   dir_path = "/home/yourname/Long-Polling"
# dir_path = "/home/qhazale/lessons/7/net/HWs/Q3/Long-Polling"
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///'+dir_path+'/DataBase.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

db = SQLAlchemy(app)
#using Marshmallow to serialize objects
ma = Marshmallow (app)

#api to show all posts
#using CORS decorator for all APIs to prevent cross origin error
@cross_origin(supports_credentials=True)
def show_posts():
    posts = PostModel.query.all()
    out = {'posts':PostModel.serialize_many(posts), 'status':'OK'}
    return jsonify(out)

app.add_url_rule('/api/posts' , view_func = show_posts )

#api to add a post
@cross_origin(supports_credentials=True)
def add_posts():
    if request.method == 'POST':
        req = request.get_json(force = True)
        title = req['title']
        description = req['description']
        print (title, description)
        new_post = PostModel (title, description)
        new_post.save()
        out = {'status':'OK'}
        return jsonify (out)
    out = {'status':'method is not post'}
    return jsonify (out)

app.add_url_rule('/api/addPost' , view_func = add_posts, methods = ['GET','POST'])

#api to delete a specific post (mentioned as url parameter)
@cross_origin(supports_credentials=True)
def delete_post (post_id):
    post = PostModel.query.get(int(post_id))
    post.delete()
    out = {'status':'OK'}
    return jsonify(out)

app.add_url_rule('/api/deletePost/<int:post_id>' , view_func = delete_post)

#api to edit a specific post (mentioned as url parameter)
@cross_origin(supports_credentials=True)
def edit_post (post_id):
    post = PostModel.query.get (int(post_id))
    req = request.get_json(force = True)
    title = req['title']
    description = req['description']
    post.edit (title, description)
    out = {'status':'OK'}
    return jsonify(out)

app.add_url_rule('/api/editPost/<int:post_id>' , view_func = edit_post, methods = ['GET','POST'])

#MAIN api to do long polling
@cross_origin(supports_credentials=True)
def poll ():
    #when the hanging get has been arived ?!
    now = datetime.now()
    while True:
        db.session.commit()
        #query all posts from db and check whether they have changed or not
        for post in PostModel.query.all() :
            print (post.timestamp)
            if (post.timestamp > now)  :
                
                out = {'status':'changed'}
                return jsonify (out)
        for log in PostLog.query.all():
            if (log.timestamp > now):
                out = {'status':'changed'}
                return jsonify (out)
        #sleep for 0.5s then loop again until a change occurs
        time.sleep (0.5)

app.add_url_rule('/api/poll' , view_func = poll)


#a model to hold all posts
#title, description and the time when the posts have been uploaded
class PostModel (db.Model):
    id = db.Column(db.Integer, primary_key = True)
    title = db.Column (db.String(40), nullable = False)
    description = db.Column (db.Text , nullable = False)
    timestamp = db.Column (db.DateTime)


    def __init__ (self , title , description):
        self.title = title
        self.description = description
        self.timestamp = datetime.now()

    def edit (self, title, description):
        self.title = title
        self.description = description
        self.timestamp = datetime.now()
        db.session.commit()

    def save (self):
        db.session.add (self)
        db.session.commit()

    def delete(self):
        new_log = PostLog(self.id)
        new_log.save()
        db.session.delete(self)
        db.session.commit()

    @staticmethod
    def serialize_many (arg):
        return PostModelSchema(many = True).dump (arg).data


#serializer class
class PostModelSchema (ma.ModelSchema):
    class Meta:
        model = PostModel

#a model to hold all deleted posts with their deletetion time
class PostLog (db.Model):
    id = db.Column(db.Integer, primary_key = True)
    deleted_post_id = db.Column(db.Integer)
    timestamp = db.Column (db.DateTime)

    def __init__ (self, deleted_post_id):
        self.deleted_post_id = deleted_post_id
        self.timestamp = datetime.now()

    def save (self):
        db.session.add (self)
        db.session.commit()

