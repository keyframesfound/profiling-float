from flask import Flask, render_template_string
from datetime import datetime
from zoneinfo import ZoneInfo  # Use zoneinfo instead of pytz

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
    hong_kong_time = datetime.now(ZoneInfo("Asia/Hong_Kong"))
    hong_kong_time_str = hong_kong_time.strftime('%Y-%m-%d %H:%M:%S')
    hong_kong_time_js = hong_kong_time.strftime('%H:%M:%S')
    graph_template = f'''
    <!doctype html>
    <html>
    <head>
        <title>Graph Generator</title>
        <script src="/static/js/chart.js"></script>
    </head>
    <body>
        <h1>Graph Generator</h1>
        <p>Current Hong Kong Time: {hong_kong_time_str}</p>
        <textarea id="dataInput" rows="10" cols="50" placeholder="Paste your comma separated values: pressure,temperature on each line"></textarea>
        <br>
        <button onclick="generateGraphs()">Generate Graphs</button>
        <div>
            <div style="margin-bottom:5px;"><b>iEngineering</b></div>
            <canvas id="graph1" width="400" height="200"></canvas>
            <div style="margin:10px 0 5px 0;"><b>iEngineering</b></div>
            <canvas id="graph2" width="400" height="200"></canvas>
        </div>
        <script>
        function generateGraphs() {{
            const lines = document.getElementById('dataInput').value.split('\\n').filter(line => line.trim() !== '');
            const pressures = [];
            const depths = [];
            const temperatures = [];
            const timeLabels = [];
            let lastTemp = 0;
            let lastPressure = 0;
            // Use time after initialise (0, 5, 10, ...) for each record (5 seconds interval)
            for (let i = 0; i < lines.length; i++) {{
                const line = lines[i];
                const parts = line.split(',');
                if(parts.length >= 2) {{
                    // Time after initialise in 5 second intervals
                    timeLabels.push((i * 5).toString());
                    let rawPressure = parseFloat(parts[0]);
                    let pressure = rawPressure / 1000;
                    if (rawPressure < 0) pressure = lastPressure;
                    else if (rawPressure > 1900) pressure = lastPressure;
                    else lastPressure = pressure;
                    pressures.push(pressure);
                    // Calculate depth in meters: depth = pressure (kPa) / 9.81
                    let depth = pressure / 9.81;
                    depths.push(depth);
                    let temp = parseFloat(parts[1]);
                    if (temp < 0 || temp > 100) temp = lastTemp;
                    else lastTemp = temp;
                    temperatures.push(temp);
                }}
            }}

            const ctx1 = document.getElementById('graph1').getContext('2d');
            const ctx2 = document.getElementById('graph2').getContext('2d');
            
            // Graph for Depth (meters)
            new Chart(ctx1, {{
                type: 'line',
                data: {{
                    labels: timeLabels,
                    datasets: [{{
                        label: 'Depth (meters)',
                        data: depths,
                        borderColor: 'rgba(255,99,132,1)',
                        fill: false,
                        tension: 0.1
                    }}]
                }},
                options: {{ 
                    responsive: true,
                    scales: {{
                        x: {{
                            title: {{
                                display: true,
                                text: 'Time after initialise (s)'
                            }}
                        }},
                        y: {{
                            title: {{
                                display: true,
                                text: 'Depth (meters)'
                            }}
                        }}
                    }}
                }}
            }});
            
            // Graph for Temperature
            new Chart(ctx2, {{
                type: 'line',
                data: {{
                    labels: timeLabels,
                    datasets: [{{
                        label: 'Temperature (°C)',
                        data: temperatures,
                        borderColor: 'rgba(54,162,235,1)',
                        fill: false,
                        tension: 0.1
                    }}]
                }},
                options: {{ 
                    responsive: true,
                    scales: {{
                        x: {{
                            title: {{
                                display: true,
                                text: 'Time after initialise (s)'
                            }}
                        }},
                        y: {{
                            title: {{
                                display: true,
                                text: 'Temperature (°C)'
                            }}
                        }}
                    }}
                }}
            }});

            // List out every data input at the bottom
            let dataListHtml = '<h3>Input Data</h3><ul style="list-style-type:none;padding-left:0;">';
            lastTemp = 0;
            lastPressure = 0;
            for (let i = 0; i < lines.length; i++) {{
                // Remove numbers after the comma and the comma itself for display
                let line = lines[i].replace(/,.*/, '');
                // Divide the data by 1000 and format to 4 significant digits for display
                let divided = '';
                let num = parseFloat(line);
                let tempMeter = 0;
                if (!isNaN(num)) {{
                    let displayPressure = num;
                    if (displayPressure < 0) displayPressure = lastPressure * 1000;
                    else if (displayPressure > 1900) displayPressure = lastPressure * 1000;
                    else lastPressure = displayPressure / 1000;
                    divided = Number(displayPressure / 1000).toPrecision(4) + ' kpa ';
                    tempMeter = Number(displayPressure / (1000*9.81)).toPrecision(4);
                }}
                // Show temperature as meter (depth = temperature / 9.81)
                if (lines[i].split(',').length >= 2) {{
                    let temp = parseFloat(lines[i].split(',')[1]);
                    if (temp < 0 || temp > 100) temp = lastTemp;
                    else lastTemp = temp;
                    
                }}
                dataListHtml += '<li>RN07 ' + timeLabels[i] + ' UTC+8 ' + divided + tempMeter + ' meters</li>';
            }}
            dataListHtml += '</ul>';
            let dataListDiv = document.getElementById('dataList');
            if (!dataListDiv) {{
                dataListDiv = document.createElement('div');
                dataListDiv.id = 'dataList';
                document.body.appendChild(dataListDiv);
            }}
            dataListDiv.innerHTML = dataListHtml;
        }}
        </script>
    </body>
    </html>
    '''
    return render_template_string(graph_template)

if __name__ == '__main__':
    # Run on all interfaces at port 3900.
    app.run(debug=False, host='0.0.0.0', port=3900)