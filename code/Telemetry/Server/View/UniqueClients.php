<html>
	<body>

Unique clients:
		<table border="1">
		<tr><td>Id</td><td>Client</td><td>Created</td></tr>
<?php
	require("../Config.php");

	// Connect to database.
	$db = new mysqli($DB_SERVER, $DB_USER, $DB_PASSWORD, $DB_NAME);
	if ($db->connect_error)
		die($db->connect_error);

	// Query all clients.
	$result = $db->query("SELECT * FROM `tbl_clients`");

	// Create row for each client.
	if ($result->num_rows > 0)
	{
		while ($row = $result->fetch_assoc())
		{
			echo("<tr>\n");
			echo("<td>" . $row["id"] . "</td>\n");
			echo("<td>" . $row["client"] . "</td>\n");
			echo("<td>" . date("c", $row["serverTimeStamp"]) . "</td>\n");
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