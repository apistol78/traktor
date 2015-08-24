<html>
	<body>

Aggregated values:
		<table border="1">
		<tr><td>Symbol</td><td>Value</td></tr>
<?php
	require("../Config.php");

	// Connect to database.
	$db = new mysqli($DB_SERVER, $DB_USER, $DB_PASSWORD, $DB_NAME);
	if ($db->connect_error)
		die($db->connect_error);

	// Get all value symbols.
	$symbols = $db->query("SELECT * FROM tbl_symbols");
	while ($symbolRow = $symbols->fetch_assoc())
	{
		// Aggregate each symbol.
		$result = $db->query("SELECT SUM(tbl_values.value) AS aggregate FROM tbl_values WHERE tbl_values.symbolId = " . $symbolRow["id"]);
		if ($result->num_rows > 0)
		{
			while ($row = $result->fetch_assoc())
			{
				echo("<tr>\n");
				echo("<td>" . $symbolRow["symbol"] . "</td>\n");
				echo("<td>" . $row["aggregate"] . "</td>\n");
				echo("</tr>\n");
			}
		}
	}

	$db->close();
?>
		</table>

	</body>
</html>