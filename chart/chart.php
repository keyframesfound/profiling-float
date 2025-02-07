<?php
// Database connection details
$servername = "localhost";
$username = "root";
$password = "ssc";
$dbname = "rov";

// Create a connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}
$uuid=$_REQUEST["uuid"];

$sql="SELECT min(d) as 'min' FROM rov_float WHERE uuid='$uuid'";

$result = $conn->query($sql);
$min=0;
while ($row = $result->fetch_assoc()) {
    $min=$row['min'];
}



// Query to fetch pressure and time data from the database
$sql = "SELECT d, TIME_FORMAT(added_on, '%H:%i:%s') as 'added_on' FROM rov_float WHERE uuid = '$uuid' ORDER BY added_on asc";

$result = $conn->query($sql);

// Initialize arrays to store data
$pressureData = array();
$timeData = array();

// Fetch data from the result set
while ($row = $result->fetch_assoc()) {
    $pressureData[] = ($row['d']+abs($min))*-1;
    $timeData[] = $row['added_on'];
}



?>


<!DOCTYPE html>
<html>
<head>
    <title>Line Graph Example</title>
    <script src="chart.js"></script>
</head>
<style>
table,th,td,tr {
  border: 1px solid;
  border-collapse: collapse;
  font-size:18px;
  text-align:center;
}
td{
	padding:3px;
}
</style>
<body>
    <canvas id="lineGraph" width="800" height="400"></canvas>

    <script>
        // Sample data
        var timeData = <?php echo json_encode($timeData);?>;
        var pressureData = <?php echo json_encode($pressureData);?>;

        // Create a new line chart
        var ctx = document.getElementById("lineGraph").getContext("2d");
        var lineChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: timeData,
                datasets: [{
                    label: 'Depth (Meters)',
                    data: pressureData,
                    borderColor: 'blue',
                    fill: false
                }]
            },
            options: {
                scales: {
                    y: {
                        beginAtZero: false,
                suggestedMin: Math.min(...pressureData), // Use the minimum value of the data as the minimum scale value
                suggestedMax: Math.max(...pressureData)  // Use the maximum value of the data as the maximum scale value
                    }
                }
            }
        });
    </script>
<h1>- Data from float -</h1>
<?php


$sql="SELECT * FROM rov_float WHERE uuid='$uuid' order by added_on asc";

$result = $conn->query($sql);
$table="<table><tr><th>Team#</th><th>Time</th><th>Pressure</th><th>Depth(meters)</th></tr>";
while ($row = $result->fetch_assoc()) {
    $teamid=explode(";",$row['ssc_id'])[1];
    $added_on=date("H:i:s",strtotime($row['added_on']));
    $pressure=$row['p']." kpa";
    $depth=round($row['d']+abs($min),2);
    $table.="<tr><td>$teamid</td><td>$added_on</td><td>$pressure</td><td>$depth</td></tr>";
    $teamname=explode(";",$row['ssc_id'])[0];
}
echo "<h2>Team Name: $teamname</h2>";

echo $table;
// Close the database connection
$conn->close();
?>

</body>
</html>