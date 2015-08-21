<?php
	require("../Config.php");

	// Extract XML data from request content.
	$xmldata = file_get_contents('php://input');
	$xmldata = urldecode($xmldata);
	if ($xmldata == null)
	{
		header("HTTP/1.1 400 No payload");
		exit;
	}

	// Parse XML document.
	$xml = simplexml_load_string($xmldata);
	if ($xml == null)
	{
		header("HTTP/1.1 400 Failed to parse payload");
		exit;
	}

	// Get values from request.
	$client = $xml->client;
	$symbol = $xml->symbol;
	$delta = $xml->delta;
	$timeStamp = $xml->timeStamp;
	if ($client == null || $symbol == null || $delta == null || $timeStamp == null)
	{
		header("HTTP/1.1 400 Invalid update");
		exit;
	}

	// Connect to database.
	$db = new mysqli($DB_SERVER, $DB_USER, $DB_PASSWORD, $DB_NAME);
	if ($db->connect_error)
	{
		header("HTTP/1.1 500 Unable to open database: ". $db->connect_error);
		exit;
	}

	// Resolve db client index.
	$result = $db->query("SELECT * FROM tbl_clients WHERE client='" . $client . "'");
	if ($result->num_rows <= 0)
	{
		// No such client, add new record.
		if ($db->query("INSERT INTO tbl_clients (client) VALUES ('" . $client . "')") !== true)
			die("Unable to add client: " . $db->error);

		$result = $db->query("SELECT * FROM tbl_clients WHERE client='" . $client . "'");
	}
	if ($result->num_rows <= 0)
		die("Unexpected error");

	$row = $result->fetch_assoc();
	$clientId = $row["id"];

	// Resolve db symbol index.
	$result = $db->query("SELECT * FROM tbl_symbols WHERE symbol='" . $symbol . "'");
	if ($result->num_rows <= 0)
	{
		// No such symbol, add new record.
		if ($db->query("INSERT INTO tbl_symbols (symbol) VALUES ('" . $symbol . "')") !== true)
			die("Unable to add symbol: " . $db->error);

		$result = $db->query("SELECT * FROM tbl_symbols WHERE symbol='" . $symbol . "'");
	}
	if ($result->num_rows <= 0)
		die("Unexpected error");

	$row = $result->fetch_assoc();
	$symbolId = $row["id"];

	// Update value in database.
	if ($db->query(
		"INSERT INTO tbl_values (clientId, symbolId, value, timeStamp) VALUES (" . $clientId . ", " . $symbolId . ", " . $delta . ", " . $timeStamp . ") " .
		"ON DUPLICATE KEY UPDATE " .
		"clientId = " . $clientId . ", " .
		"symbolId = " . $symbolId . ", " .
		"value = value + " . $delta . ", " .
		"timeStamp = " . $timeStamp
	) !== true)
		die("Unable to update value: " . $db->error);

	// Response through status code.
	header("HTTP/1.1 200");
?>
