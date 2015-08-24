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
	$clientTimeStamp = $xml->timeStamp;
	$serverTimeStamp = time();

	// Mandatory check.
	if ($client == null || $symbol == null || $clientTimeStamp == null || $serverTimeStamp == null)
	{
		header("HTTP/1.1 400 Invalid event");
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

	// Insert event in database.
	if ($db->query(
		"INSERT INTO tbl_events (clientId, symbolId, serverTimeStamp, clientTimeStamp) VALUES (" . $clientId . ", " . $symbolId . ", " . $serverTimeStamp . ", " . $clientTimeStamp . ")"
	) !== true)
		die("Unable to insert event: " . $db->error);

	// Response through status code.
	header("HTTP/1.1 200 OK");
?>
