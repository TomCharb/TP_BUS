from flask import Flask
from flask import render_template
from flask import request
from flask import jsonify
import serial
import json

tab_T = [] #tableau des températures
tab_P = [] #tableau des pressions

app = Flask(__name__)

##==========================TP3==============================

@app.route('/')
def hello_world():
    return 'Hello, World!\n'

welcome = "Welcome to 3ESE API!"

@app.route('/api/welcome/', methods=['GET', 'POST','DELETE'])
def api_welcome():
        global welcome
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "args": request.args,
            "headers": dict(request.headers),
    }
        if request.method == 'POST':
                data = request.get_json()
                if 'message' in data:
                        welcome = data['message']
                        return welcome
        if request.method == 'GET':
                return welcome
        if request.method == 'DELETE':
                welcome =''
                return welcome

@app.route('/api/welcome/<int:index>', methods=['GET', 'POST','PUT','PATCH','DELETE'])
def api_welcome_index(index=None):
        global welcome
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "index" : index,
            "args": request.args,
            "headers": dict(request.headers),
    }
        if request.method == 'POST':
                welcome = request.get_json()
                return welcome
        if request.method == 'GET':
                if index == None:
                        return welcome
                elif index > 19 or index < 0 :
                        return abort(404)
                else:
                        return jsonify({"index": index, "val": welcome[index]})
        if request.method =='PUT':
                data = request.get_json()
                if 'word' in data:
                        word_insert = data['word']
                        if 0 <= index <= len(welcome):
                                welcome = welcome[:index]+word_insert+welcome[index:]
                                return welcome
        if request.method =='PATCH':
                data = request.get_json()
                if 'letter' in data:
                        letter_insert = data['letter']
                        if 0 <= index <= len(welcome):
                                welcome = welcome[:index]+letter_insert+welcome[index+1:]
                                return welcome
        if request.method =='DELETE':
                if 0 <= index <= len(welcome):
                        welcome = welcome[:index]+welcome[index+1:]
                        return welcome


@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404
methods=['GET', 'POST']
@app.route('/api/request/', methods=['GET', 'POST'])
@app.route('/api/request/<path>', methods=['GET','POST'])
def api_request(path=None):
    resp = {
            "method":   request.method,
            "url" :  request.url,
            "path" : path,
            "args": request.args,
            "headers": dict(request.headers),
    }
    if request.method == 'POST':
        resp["POST"] = {
                "data" : request.get_json(),
                }
    return jsonify(resp)

##===================================TP5=============================================

ser = serial.Serial("/dev/ttyAMA0",115200,timeout=1)
ser.reset_output_buffer()
ser.reset_input_buffer()
#temperature
@app.route('/api/temp/', methods=['GET', 'POST'])
def api_temp():
        ser.reset_output_buffer()
        ser.reset_input_buffer()
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "args": request.args,
            "headers": dict(request.headers),
    }
        if request.method == 'POST' :
                ser.write(b'GET_T')             #envoie à la stm32 que l'on veut faire un GET_T
                tempo = ser.readline().decode() #récupérer la valeur envoyée par la stm32
                tab_T.append(tempo[:9])         #retire le \r\n et ajoute dans le tableau
                return jsonify(tab_T[-1])       #renvoie la valeur
        if request.method == 'GET' :
                return jsonify(tab_T)           #renvoie le tableau

@app.route('/api/temp/<int:index>', methods=['GET','DELETE'])
def api_temp_index(index=None):
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "index" : index,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'GET' :
                if (index < len(tab_T)):
                        return jsonify(tab_T[index])    #récupère la valeur du tableau à l'index
                else :
                        return jsonify("error : index out of range")
        if request.method == 'DELETE' :
                if (index < len(tab_T)):
                        return jsonify("la valeur "+tab_T.pop(index)+ " a été retirée") #récupère la valeur du tableau à l'index et supprime du tableau
                else :
                        return jsonify("error : index out of range")


#pression
@app.route('/api/pres/', methods=['GET', 'POST'])
def api_pres():
        ser.reset_output_buffer()
        ser.reset_input_buffer()
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'POST' :
                ser.write(b'GET_P')             #envoie à la stm32 que l'on veut faire un GET_P
                tempo = ser.readline().decode() #récupérer la valeur envoyée par la stm32
                tab_P.append(tempo[:20])        #retire le \r\n et ajoute dans le tableau
                return jsonify(tab_P[-1])       #renvoie la valeur
        if request.method == 'GET' :
                return jsonify(tab_P)           #renvoie le tableau


@app.route('/api/pres/<int:index>', methods=['GET', 'DELETE'])
def api_pres_index(index=None):
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "index" : index,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'GET' :
                if (index < len(tab_P)):
                        return jsonify(tab_P[index])    #récupère la valeur du tableau à l'index
                else :
                        return jsonify("error : index out of range")
        if request.method == 'DELETE' :
                if (index < len(tab_P)):
                        return jsonify("la valeur "+tab_P.pop(index)+ " a été retirée") #récupère la valeur du tableau à l'index et supprime du tableau
                else :
                        return jsonify("error : index out of range")

#scale
@app.route('/api/scale/', methods=['GET'])
def api_scale():
        ser.reset_output_buffer()
        ser.reset_input_buffer()

        resp = {
            "method":   request.method,
            "url" :  request.url,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'GET' :
                ser.write(b'GET_K')     #envoie à la stm32 que l'on veut faire un GET_K
                tempo = ser.readline().decode() #récupérer la valeur envoyée par la stm32
                return jsonify(tempo[:3])       #retire le \r\n et renvoie la valeur

@app.route('/api/scale/<int:index>', methods=['POST'])
def api_scale_index(index=None):
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "index" : index,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'POST' :
                ser.write(b'SET_K')     #envoie à la stm32 que l'on veut faire un SET_K
                ser.write(index)        #envoie à la stm32 la nouvelle veleur
                return jsonify("le nouveau coeff est "+ str(index))

#angle
@app.route('/api/angle/', methods=['GET'])
def api_angle():
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'GET' :
                return jsonify("work in progress")      #angle non récupéré pas eu le temps de le faire
