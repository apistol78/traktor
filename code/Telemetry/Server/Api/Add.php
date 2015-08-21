<?php
	require("../Config.php");
	require("../Common.php");

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
	$clientTimeStamp = $xml->timeStamp;
	$serverTimeStamp = time();

	// Mandatory check.
	if ($client == null || $symbol == null || $delta == null || $clientTimeStamp == null || $serverTimeStamp == null)
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

	// Resolve indices.
	$clientId = getClientId($db, $client);
	$symbolId = getSymbolId($db, $symbol);

	// Update value in database.
	if ($db->query(
		"INSERT INTO tbl_values (clientId, symbolId, value, serverTimeStamp, clientTimeStamp) VALUES (" . $clientId . ", " . $symbolId . ", " . $delta . ", " . $serverTimeStamp . ", " . $clientTimeStamp . ") " .
		"ON DUPLICATE KEY UPDATE " .
		"clientId = " . $clientId . ", " .
		"symbolId = " . $symbolId . ", " .
		"value = value + " . $delta . ", " .
		"serverTimeStamp = " . $serverTimeStamp . ", " .
		"clientTimeStamp = " . $clientTimeStamp
	) !== true)
		die("Unable to update value: " . $db->error);

	// Response through status code.
	header("HTTP/1.1 200");
?>
