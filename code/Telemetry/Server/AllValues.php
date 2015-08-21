<html>
	<body>

All values:
		<table border="1">
<?php
	require("Config.php");

	// Connect to database.
	$db = new mysqli($DB_SERVER, $DB_USER, $DB_PASSWORD, $DB_NAME);
	if ($db->connect_error)
		die($db->connect_error);

	// Query all clients.
	$result = $db->query(
		"SELECT tbl_clients.client, tbl_symbols.symbol, tbl_values.value, tbl_values.timeStamp FROM tbl_values " .
		"INNER JOIN tbl_clients ON tbl_values.clientId=tbl_clients.id " .
		"INNER JOIN tbl_symbols ON tbl_values.symbolId=tbl_symbols.id "
	);

	// Create row for each client.
	if ($result->num_rows > 0)
	{
		while ($row = $result->fetch_assoc())
		{
			echo("<tr>\n");
			echo("<td>" . $row["client"] . "</td>\n");
			echo("<td>" . $row["symbol"] . "</td>\n");
			echo("<td>" . $row["value"] . "</td>\n");
			echo("<td>" . date("c", $row["timeStamp"]) . "</td>\n");
			echo("</tr>\n");
		}
	}
	else
		echo("<!-- No rows -->\n");

	$db->close();

?>
		</table>

	</body>
</html>