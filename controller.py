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

# New route for graph generation
@app.route('/graph', methods=['GET'])
def graph():
    graph_template = '''
    <!doctype html>
    <html>
    <head>
        <title>Graph Generator</title>
        <!-- Changed to load Chart.js locally -->
        <script src="/static/js/chart.js"></script>
    </head>
    <body>
        <h1>Graph Generator</h1>
        <textarea id="dataInput" rows="10" cols="50" placeholder="Paste your comma separated values: pressure,temperature on each line"></textarea>
        <br>
        <button onclick="generateGraphs()">Generate Graphs</button>
        <div>
            <canvas id="graph1" width="400" height="200"></canvas>
            <canvas id="graph2" width="400" height="200"></canvas>
        </div>
        <script>
        function generateGraphs() {
            const lines = document.getElementById('dataInput').value.split('\\n').filter(line => line.trim() !== '');
            const pressures = [];
            const temperatures = [];
            const timeLabels = [];
            
            lines.forEach((line, index) => {
                const parts = line.split(',');
                if(parts.length >= 2) {
                    const seconds = index;
                    const minutes = Math.floor(seconds / 60);
                    const remainingSeconds = seconds % 60;
                    timeLabels.push(`${minutes}:${remainingSeconds.toString().padStart(2, '0')}`);
                    pressures.push(parseFloat(parts[0]));
                    temperatures.push(parseFloat(parts[1]));
                }
            });

            const ctx1 = document.getElementById('graph1').getContext('2d');
            const ctx2 = document.getElementById('graph2').getContext('2d');
            
            // Graph for Pressure
            new Chart(ctx1, {
                type: 'line',
                data: {
                    labels: timeLabels,
                    datasets: [{
                        label: 'Pressure (dbar)',
                        data: pressures,
                        borderColor: 'rgba(255,99,132,1)',
                        fill: false,
                        tension: 0.1
                    }]
                },
                options: { 
                    responsive: true,
                    scales: {
                        x: {
                            title: {
                                display: true,
                                text: 'Time (MM:SS)'
                            }
                        },
                        y: {
                            title: {
                                display: true,
                                text: 'Pressure (dbar)'
                            }
                        }
                    }
                }
            });
            
            // Graph for Temperature
            new Chart(ctx2, {
                type: 'line',
                data: {
                    labels: timeLabels,
                    datasets: [{
                        label: 'Temperature (°C)',
                        data: temperatures,
                        borderColor: 'rgba(54,162,235,1)',
                        fill: false,
                        tension: 0.1
                    }]
                },
                options: { 
                    responsive: true,
                    scales: {
                        x: {
                            title: {
                                display: true,
                                text: 'Time (MM:SS)'
                            }
                        },
                        y: {
                            title: {
                                display: true,
                                text: 'Temperature (°C)'
                            }
                        }
                    }
                }
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