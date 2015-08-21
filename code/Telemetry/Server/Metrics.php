<html>
	<body>

<?php
	require("Config.php");

	// Connect to database.
	$db = new mysqli($DB_SERVER, $DB_USER, $DB_PASSWORD, $DB_NAME);
	if ($db->connect_error)
		die($db->connect_error);

	$timeStampTo = time();
	$timeStampFrom = $timeStampTo - (60 * 60 * 24);

	// Calculate "Total User Count".
	$result = $db->query(
		"SELECT COUNT(*) AS tuc FROM tbl_clients"
	);
	$row = $result->fetch_assoc();
	$TUC = $row["tuc"];

	// Calculate "Daily Active Users".
	$result = $db->query(
		"SELECT COUNT(DISTINCT clientId) AS dau FROM tbl_values " .
		"WHERE tbl_values.serverTimeStamp >= " . $timeStampFrom . " AND tbl_values.serverTimeStamp <= " . $timeStampTo
	);
	$row = $result->fetch_assoc();
	$DAU = $row["dau"];

	// Calculate "Daily New Users".
	$result = $db->query(
		"SELECT COUNT(*) AS dnu FROM tbl_clients " .
		"WHERE tbl_clients.serverTimeStamp >= " . $timeStampFrom . " AND tbl_clients.serverTimeStamp <= " . $timeStampTo
	);
	$row = $result->fetch_assoc();
	$DNU = $row["dnu"];

	// Calculate "Daily Sessions".
	$result = $db->query(
		"SELECT SUM(tbl_values.value) AS ds FROM tbl_values " .
		"WHERE tbl_values.symbolId=(SELECT id FROM tbl_symbols WHERE symbol='" . $SYMBOL_LAUNCH_COUNT . "') AND tbl_values.serverTimeStamp >= " . $timeStampFrom . " AND tbl_values.serverTimeStamp <= " . $timeStampTo
	);
	$row = $result->fetch_assoc();
	$DS = $row["ds"];

	// Close connection.
	$db->close();
?>

	Total Number of Users: <?php echo($TUC); ?><br>
	Daily New Users: <?php echo($DNU); ?><br>
	Daily Active Users: <?php echo($DAU); ?><br>
	Daily Sessions: <?php echo($DS); ?><br>
	DS/DAU: <?php echo($DS/$DAU); ?><br>

	</body>
</html>