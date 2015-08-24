<html>
	<body>

<?php
	require("Config.php");

	// Connect to database.
	$db = new mysqli($DB_SERVER, $DB_USER, $DB_PASSWORD, $DB_NAME);
	if ($db->connect_error)
		die($db->connect_error);

	// Drop events which are older than limit.
	$timeStampLimit = time() - $EVENT_LIMIT;
	$db->query(
		"DELETE FROM tbl_events WHERE tbl_events.serverTimeStamp <= " . $timeStampLimit
	);	

	// Close connection.
	$db->close();
?>

	Database cleaned.

	</body>
</html>
