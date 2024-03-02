from flask import Flask, make_response, render_template, redirect, url_for, flash, request
from time import time

app = Flask(__name__)
app.secret_key = b'_5#y2L"F4Q8z\n\xec]/'

KEEP_LAST_SENT_TIME = 20
last_sent_id = ''
last_sent_time = 0


class UserDB():
    def __init__(self, path):
        self.path = path
        self.users = {}

        # create file if does not exist
        f = open(path, 'a+')
        f.write('')
        f.close()

    def readDB(self):
        self.users = {}
        with open(self.path) as access_list:
            for line in access_list:
                parts = line.split();
                if (len(parts) != 2):
                    raise Exception('falsches Format der access-list')
                uid = parts[0]
                pin = parts[1]

                self.users[uid] = pin

    def writeDB(self):
        with open(self.path, "w") as access_list:
            for uid, pin in self.users.items():
                access_list.write(f"{uid} {pin}\n")

    def get_users(self):
        self.readDB()
        return self.users.keys()

    def add_user(self, uid, pin):
        self.readDB()
        if self.users.get(uid) != None:
            raise Exception('ID bereits vorhanden')
        self.users[uid] = pin
        self.writeDB()

    def remove_user(self, uid):
        self.readDB()
        if self.users.get(uid) == None:
            raise Exception('ID nicht gefunden')
        self.users.pop(uid)
        self.writeDB()

    def login(self, uid, pin):
        self.readDB()
        return self.users.get(uid) == pin


userDB = UserDB('accesslist.txt')


@app.route("/")
def main():
    ids = []
    try:
        ids = userDB.get_users()
    except Exception as e:
        flash(e)

    return render_template('index.html', ids = ids)

@app.route('/login')
def login():
    user_id = request.args.get("id")
    user_pin = request.args.get("pin")

    global last_sent_time
    last_sent_time = time()
    global last_sent_id
    last_sent_id = user_id

    loginGranted = False
    try:
        loginGranted = userDB.login(user_id, user_pin)
    except Exception as e:
        return make_response(f'access denied! ({str(e)})', 401)

    if loginGranted:
        return make_response('access granted!', 200)
    return make_response('access denied!', 401)

@app.route("/add_user")
def add_user():
    global last_sent_id

    new_id = request.args.get("id")
    new_pwd = request.args.get("pwd")

    if new_id and new_pwd:
        try:
            userDB.add_user(new_id, new_pwd)
            flash('Nutzer-ID wurde hinzugefügt')
        except Exception as e:
            flash(str(e))

        last_sent_id = ''
        return redirect(request.path)

    if time() - last_sent_time > KEEP_LAST_SENT_TIME:
        last_sent_id = ''

    return render_template('add_user.html', sent_id = last_sent_id)

@app.route("/remove_user")
def remove_user():
    global last_sent_id

    user_id = request.args.get("id")

    if user_id:
        try:
            userDB.remove_user(user_id)
            flash('Nutzer-ID wurde gelöscht')
        except Exception as e:
            flash(str(e))

        last_sent_id = ''
        return redirect(request.path)

    if time() - last_sent_time > KEEP_LAST_SENT_TIME:
        last_sent_id = ''

    return render_template('remove_user.html', sent_id = last_sent_id)

@app.route("/help")
def help():
    return render_template('help.html')


if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
