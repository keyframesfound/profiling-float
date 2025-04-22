from flask import Flask, render_template_string

app = Flask(__name__)

# Updated main page template with modified third button.
html_template = '''
<!doctype html>
<html>
<head>
    <title>Button Navigation</title>
    <style>
      button { display: block; margin: 10px; padding: 10px; font-size: 16px; }
    </style>
</head>
<body>
    <h1>Float Control Pannel</h1>
    <!-- Button to open Sink Float page -->
    <button onclick="window.open('http://192.168.4.1/control', '_blank');">Sink Float</button>
    <!-- Button to open Float Pressure Data page -->
    <button onclick="window.open('http://192.168.4.1/data', '_blank');">Float Pressure Data</button>
    <!-- Third button now opens Graph Generator page -->
    <button onclick="window.open('/graph', '_blank');">Generate Graph</button>
</body>
</html>
'''

@app.route('/')
def home():
    return render_template_string(html_template)

# Updated graph_template to show Depth (m) instead of pressure and display time in GMT+8.
@app.route('/graph', methods=['GET'])
def graph():
    graph_template = '''
    <!doctype html>
    <html>
    <head>
        <title>iEngineer</title>
        <!-- Changed to load Chart.js locally -->
        <script src="/static/js/chart.js"></script>
    </head>
    <body>
        <h1>iEngineer</h1>
        <p id="gmtTime"></p>
        <textarea id="dataInput" rows="10" cols="50" placeholder="Paste your comma separated values: depth,temperature,GMT Time on each line"></textarea>
        <br>
        <button onclick="generateGraphs()">Generate Graphs</button>
        <div>
            <canvas id="graphDepth" width="400" height="200"></canvas>
            <canvas id="graphTemp" width="400" height="200"></canvas>
        </div>
        <script>
        function generateGraphs() {
            const lines = document.getElementById('dataInput').value.split('\\n').filter(line => line.trim() !== '');
            let currentGMT = "";
            const depths = [];
            const temperatures = [];
            const gmtTimes = [];
            lines.forEach(line => {
                const parts = line.split(',');
                if (parts[0].trim().startsWith("GMT Time:")) {
                    currentGMT = parts[0].replace("GMT Time:", "").trim();
                    document.getElementById('gmtTime').innerText = "Current GMT: " + currentGMT;
                } else if(parts.length >= 3) {
                    depths.push(parseFloat(parts[0]));
                    temperatures.push(parseFloat(parts[1]));
                    gmtTimes.push(currentGMT);
                }
            });
            // Graph for Depth (m) using GMT time as labels
            new Chart(document.getElementById('graphDepth').getContext('2d'), {
                type: 'line',
                data: { labels: gmtTimes, datasets: [{ label: 'Depth (m)', data: depths, borderColor: 'rgba(255,99,132,1)', fill: false, tension: 0.1 }] },
                options: { responsive: true, scales: { x: { title: { display: true, text: 'GMT Time' } }, y: { title: { display: true, text: 'Depth (m)' } } } }
            });
            // Graph for Temperature (°C) using GMT time as labels
            new Chart(document.getElementById('graphTemp').getContext('2d'), {
                type: 'line',
                data: { labels: gmtTimes, datasets: [{ label: 'Temperature (°C)', data: temperatures, borderColor: 'rgba(54,162,235,1)', fill: false, tension: 0.1 }] },
                options: { responsive: true, scales: { x: { title: { display: true, text: 'GMT Time' } }, y: { title: { display: true, text: 'Temperature (°C)' } } } }
            });
        }
        </script>
    </body>
    </html>
    '''
    return render_template_string(graph_template)

if __name__ == '__main__':
    # Run on all interfaces at port 3900.
    app.run(debug=False, host='0.0.0.0', port=3900)