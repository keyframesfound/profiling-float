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
    <h1>Select a Page</h1>
    <!-- Button to open a specific IP webpage -->
    <button onclick="window.location.href='http://192.168.1.101';">Open IP Web Page</button>
    
    <!-- Button to open a specific webpage -->
    <button onclick="window.location.href='http://example.com';">Open Example.com</button>
    
    <!-- Third button that opens another specific webpage -->
    <button onclick="window.location.href='http://anotherpage.com';">Open Another Page</button>
</body>
</html>
'''

@app.route('/')
def home():
    return render_template_string(html_template)

if __name__ == '__main__':
    # Run on all interfaces at port 5000.
    app.run(debug=True, host='0.0.0.0', port=5000)