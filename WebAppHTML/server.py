# # app.py
# from flask import Flask, request, jsonify
# import openai
# import os
# from dotenv import load_dotenv

# load_dotenv()
# openai.api_key = os.getenv("OPENAI_API_KEY")

# app = Flask(__name__)

# @app.route('/chat', methods=['POST'])
# def chat():
#     data = request.get_json()
#     user_message = data['message']

#     response = openai.ChatCompletion.create(
#         model="gpt-3.5-turbo",
#         messages=[{ "role": "user", "content": user_message }]
#     )

#     reply = response['choices'][0]['message']['content']
#     return jsonify({ "reply": reply })

# if __name__ == '__main__':
#     app.run(debug=True)


from flask import Flask, request, jsonify, render_template
from openai import OpenAI
import os
from dotenv import load_dotenv

load_dotenv()
client = OpenAI(api_key=os.getenv("OPENAI_API_KEY"))

app = Flask(__name__)

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/mqtt_app')
def mqtt_app():
    return render_template('mqtt_app.htm')

@app.route('/mqtt_monitor')
def mqtt_monitor():
    return render_template('mqtt_monitor.htm')

@app.route('/chat', methods=['POST'])
def chat():
    data = request.get_json()
    user_message = data['message']

    response = client.chat.completions.create(
    model="gpt-3.5-turbo",
    messages=[{ "role": "user", "content": user_message }]
    )

    reply = response.choices[0].message.content
    return jsonify({ "reply": reply })

if __name__ == '__main__':
    app.run(debug=True)
