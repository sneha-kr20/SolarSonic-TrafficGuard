<?php
date_default_timezone_set('Asia/Kolkata');

function connectToDatabase() {
    $hostname = "localhost";
    $username = "root";
    $password = "";
    $database = "sensordata";

    $conn = new mysqli($hostname, $username, $password, $database);

    // Check the connection
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }

    return $conn;
}

// Function to insert data into the database
function insertData($conn, $temperature, $humidity, $mq2_analog, $mq2_digital, $sound_analog, $sound_digital, $mq9_analog, $mq9_digital, $mq8_analog, $mq8_digital, $pm25_density, $pm10_density) {
    $datatime = date('Y-m-d H:i:s'); // Assuming datatime is in the format YYYY-MM-DD HH:MM:SS

    // Check for null values
    if ($temperature === null || $humidity === null) {
        // Create an array with the received data
        $responseData = array(
            'error' => 'Temperature or humidity is null',
            'received_data' => $_POST
        );
    
        // Encode the array as JSON
        $jsonResponse = json_encode($responseData);
    
        // Send the JSON response back to the client (ESP8266)
        echo $jsonResponse;
        return;
    }
    
	
    // Use prepared statements to prevent SQL injection
    $sql = "INSERT INTO dht11 (temperature, humidity, mq2_analog, mq2_digital, sound_analog, sound_digital, mq9_analog, mq9_digital, mq8_analog, mq8_digital, pm25_density, pm10_density, datatime) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    $stmt = $conn->prepare($sql);

    // Bind parameters
    $stmt->bind_param("ddsddddddddds", $temperature, $humidity, $mq2_analog, $mq2_digital, $sound_analog, $sound_digital, $mq9_analog, $mq9_digital, $mq8_analog, $mq8_digital, $pm25_density, $pm10_density,  $datatime);

    if ($stmt->execute()) {
        echo "Data inserted successfully";
    } else {
        // Send error message back to the client (ESP8266)
        $errorResponse = array(
            'error' => 'Failed to insert data into the database',
            'error_message' => $stmt->error
        );
        echo json_encode($errorResponse);
    }

    $stmt->close();
}

// Retrieve data from the ESP8266 and trim whitespace characters
$temperature = isset($_POST['temperature']) ? trim($_POST['temperature']) : null;
$humidity = isset($_POST['humidity']) ? trim($_POST['humidity']) : null;
$mq2_analog = isset($_POST['mq2_analog']) ? trim($_POST['mq2_analog']) : null;
$mq2_digital = isset($_POST['mq2_digital']) ? trim($_POST['mq2_digital']) : null;
$sound_analog = isset($_POST['sound_analog']) ? trim($_POST['sound_analog']) : null;
$sound_digital = isset($_POST['sound_digital']) ? trim($_POST['sound_digital']) : null;
$mq9_analog = isset($_POST['mq9_analog']) ? trim($_POST['mq9_analog']) : null;
$mq9_digital = isset($_POST['mq9_digital']) ? trim($_POST['mq9_digital']) : null;
$mq8_analog = isset($_POST['mq8_analog']) ? trim($_POST['mq8_analog']) : null;
$mq8_digital = isset($_POST['mq8_digital']) ? trim($_POST['mq8_digital']) : null;
$pm25_density = isset($_POST['dust_density_pm25']) ? trim($_POST['dust_density_pm25']) : null;
$pm10_density = isset($_POST['dust_density_pm10']) ? trim($_POST['dust_density_pm10']) : null;

// Insert data into the database
$connection = connectToDatabase();

if ($connection) {
    insertData($connection, $temperature, $humidity, $mq2_analog, $mq2_digital, $sound_analog, $sound_digital, $mq9_analog, $mq9_digital, $mq8_analog, $mq8_digital,$pm25_density, $pm10_density);
    $connection->close(); // Close the connection when done
} else {
    // Send connection error message back to the client (ESP8266)
    $errorResponse = array(
        'error' => 'Failed to connect to the database'
    );
    echo json_encode($errorResponse);
}

?>
