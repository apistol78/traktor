<?php

function getClientId($db, $client)
{
	// Resolve db client index.
	$result = $db->query("SELECT * FROM tbl_clients WHERE client='" . $client . "'");
	if ($result->num_rows <= 0)
	{
		// No such client, add new record.
		if ($db->query("INSERT INTO tbl_clients (client, serverTimeStamp) VALUES ('" . $client . "', " . time() . ")") !== true)
			die("Unable to add client: " . $db->error);

		$result = $db->query("SELECT * FROM tbl_clients WHERE client='" . $client . "'");
	}
	if ($result->num_rows <= 0)
		die("Unexpected error");

	$row = $result->fetch_assoc();
	return $row["id"];	
}

function getSymbolId($db, $symbol)
{
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
	return $row["id"];
}

?>