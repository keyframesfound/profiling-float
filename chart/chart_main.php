<!DOCTYPE html>
<html>
<head>
  <title>Page Loader</title>
  <script src="jquery.js"></script>
</head>
<body>
  <h1>Select a page to load:</h1>
  <select id="pageSelect">
    <option value="">Select Page</option>
  <?php
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


$sql="SELECT uuid, min(added_on) as added_on FROM rov_float group by uuid";

$result = $conn->query($sql);
$uuid_array=array();
while ($row = $result->fetch_assoc()) {
    $uuid_array[$row['uuid']]=$row['added_on'];
}
foreach ($uuid_array as $k=>$v){
  echo "<option value='$k'>$v $k</option>";
}
  ?>
  </select>

  <div id="content"></div>

  <script>
    $(document).ready(function() {
      // Listen for changes in the selection list
      $("#pageSelect").change(function() {
        var selectedPage = $(this).val();
        console.log(selectedPage);
        if (selectedPage !== "") {
          // Load the selected page into the content div
          $("#content").load("chart.php?uuid="+selectedPage);
        } else {
          // Clear the content div if no page is selected
          $("#content").empty();
        }
      });
    });
  </script>
</body>
</html>