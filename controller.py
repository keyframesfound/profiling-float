from flask import Flask, render_template_string

app = Flask(__name__)

html_template = '''
<!doctype html>
<html>
<head>
    <title>Button Navigation</title>
    <style>
      button {
          display: block;
          margin: 10px;
          padding: 10px;
          font-size: 16px;
      }
    </style>
</head>
<body>
    <h1>Float Control Pannel</h1>
    <!-- Button to open a specific IP webpage -->
    <button onclick="window.location.href='http://192.168.1.101';">Sink Float</button>
    
    <!-- Button to open a specific webpage -->
    <button onclick="window.location.href='http://192.168.1.101';"Float Pressure Data</button>
    
    <!-- Third button that opens another specific webpage -->
    <button onclick="window.location.href='http://192.168.1.101';">Generate Graph</button>
</body>
</html>
'''

@app.route('/')
def home():
    return render_template_string(html_template)

if __name__ == '__main__':
    # Run on all interfaces at port 3900.
    app.run(debug=False, host='0.0.0.0', port=3900)