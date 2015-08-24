<html>
	<head>
		<title>Metrics</title>
		<style type="text/css">
body {
	font-family: arial, helvetica, serif;
	color: #f0f0f0;
	background-color: black;
}
		</style>
	</head>
	<body>
<?php
	require("../Config.php");

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
		"SELECT COUNT(DISTINCT clientId) AS dau FROM tbl_events " .
		"WHERE tbl_events.symbolId=(SELECT id FROM tbl_symbols WHERE symbol='" . $SYMBOL_LAUNCH_EVENT . "') AND tbl_events.serverTimeStamp >= " . $timeStampFrom . " AND tbl_events.serverTimeStamp <= " . $timeStampTo
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
		"SELECT COUNT(*) AS ds FROM tbl_events " .
		"WHERE tbl_events.symbolId=(SELECT id FROM tbl_symbols WHERE symbol='" . $SYMBOL_LAUNCH_EVENT . "') AND tbl_events.serverTimeStamp >= " . $timeStampFrom . " AND tbl_events.serverTimeStamp <= " . $timeStampTo
	);
	$row = $result->fetch_assoc();
	$DS = $row["ds"];

	// Close connection.
	$db->close();
?>
		<center>
			<table>
				<tr>
					<td width="200">Total Number of Users</td><td><?php echo($TUC); ?></td>
				</tr>
				<tr>
					<td>Daily New Users</td><td><?php echo($DNU); ?></td>
				</tr>
				<tr>
					<td>Daily Active Users</td><td><?php echo($DAU); ?></td>
				</tr>
				<tr>
					<td>Daily Sessions</td><td><?php echo($DS); ?></td>
				</tr>
				<tr>
					<td>DS/DAU</td><td><?php echo($DS/$DAU); ?></td>
				</tr>
			</table>
		</center>
	</body>
</html>