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
	$timeStampFrom = $timeStampTo - $SECS_PER_DAY;
	$timeStampFromMonth = $timeStampTo - 28 * $SECS_PER_DAY;

	// Get launch symbol identifier.
	$result = $db->query("SELECT id FROM tbl_symbols WHERE symbol='" . $SYMBOL_LAUNCH_EVENT . "'");
	$row = $result->fetch_assoc();
	$launchSymbolId = $row["id"];

	// Calculate "Total User Count".
	$result = $db->query(
		"SELECT COUNT(*) AS tuc FROM tbl_clients"
	);
	$row = $result->fetch_assoc();
	$TUC = $row["tuc"];

	// Calculate "Daily Active Users".
	$result = $db->query(
		"SELECT COUNT(DISTINCT clientId) AS dau FROM tbl_events " .
		"WHERE tbl_events.symbolId=" . $launchSymbolId . " AND tbl_events.serverTimeStamp >= " . $timeStampFrom . " AND tbl_events.serverTimeStamp <= " . $timeStampTo
	);
	$row = $result->fetch_assoc();
	$DAU = $row["dau"];

	// Calculate "Monthly Active Users".
	$result = $db->query(
		"SELECT COUNT(DISTINCT clientId) AS mau FROM tbl_events " .
		"WHERE tbl_events.symbolId=" . $launchSymbolId . " AND tbl_events.serverTimeStamp >= " . $timeStampFromMonth . " AND tbl_events.serverTimeStamp <= " . $timeStampTo
	);
	$row = $result->fetch_assoc();
	$MAU = $row["mau"];

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
		"WHERE tbl_events.symbolId=" . $launchSymbolId . " AND tbl_events.serverTimeStamp >= " . $timeStampFrom . " AND tbl_events.serverTimeStamp <= " . $timeStampTo
	);
	$row = $result->fetch_assoc();
	$DS = $row["ds"];

	// Calculate retentions.
	function retention($db, $launchSymbolId, $daysSinceInstall)
	{
		require("../Config.php");

		$count = 0;
		$value = 0;

		$T = time();

		$clientsFromTimeStamp = $T - ($daysSinceInstall + 1) * $SECS_PER_DAY;
		$clientsToTimeStamp = $T - $daysSinceInstall * $SECS_PER_DAY;
		// $activeFromTimeStamp = $T - $daysSinceInstall * $SECS_PER_DAY;
		$activeFromTimeStamp = $T - $SECS_PER_DAY;
		$activeToTimeStamp = $T;

		$result = $db->query("SELECT * FROM tbl_clients WHERE tbl_clients.serverTimeStamp >= " . $clientsFromTimeStamp . " AND tbl_clients.serverTimeStamp <= " . $clientsToTimeStamp);
		while ($row = $result->fetch_assoc())
		{
			$clientId = $row["id"];

			$result2 = $db->query(
				"SELECT COUNT(*) AS value FROM tbl_events " .
				"WHERE tbl_events.clientId=" . $clientId . " AND " .
				"tbl_events.symbolId=" . $launchSymbolId . " AND " .
				"tbl_events.serverTimeStamp >= " . $activeFromTimeStamp . " AND " .
				"tbl_events.serverTimeStamp <= " . $activeToTimeStamp
			);

			$row2 = $result2->fetch_assoc();
			$value += ($row2["value"] > 0 ? 1 : 0);
			$count += 1;
		}

		if ($count > 0)
			$value /= $count;

		return array(
			"value" => $value,
			"count" => $count
		);
	}

	$D1 = retention($db, $launchSymbolId, 1);
	$D3 = retention($db, $launchSymbolId, 3);
	$D7 = retention($db, $launchSymbolId, 7);
	$D30 = retention($db, $launchSymbolId, 30);

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
					<td>Monthly Active Users</td><td><?php echo($MAU); ?></td>
				</tr>
				<tr>
					<td>Daily Sessions</td><td><?php echo($DS); ?></td>
				</tr>
				<tr>
					<td>DS/DAU</td><td><?php echo(round($DS/$DAU, 2)); ?></td>
				</tr>
				<tr>
					<td>DAU/MAU</td><td><?php echo(round(100 * $DAU/$MAU, 2) . "%"); ?></td>
				</tr>
				<tr>
					<td>Retention D1</td><td><?php echo(round(100 * $D1["value"], 2) . "% of " . $D1["count"] . " client(s)"); ?></td>
				</tr>
				<tr>
					<td>Retention D3</td><td><?php echo(round(100 * $D3["value"], 2) . "% of " . $D3["count"] . " client(s)"); ?></td>
				</tr>
				<tr>
					<td>Retention D7</td><td><?php echo(round(100 * $D7["value"], 2) . "% of " . $D7["count"] . " client(s)"); ?></td>
				</tr>
				<tr>
					<td>Retention D30</td><td><?php echo(round(100 * $D30["value"], 2) . "% of " . $D30["count"] . " client(s)"); ?></td>
				</tr>
			</table>
		</center>
	</body>
</html>