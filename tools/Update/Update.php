<?xml version="1.0" encoding="utf-8"?>
<object type="traktor.update.Bundle">
	<version>1</version>
	<description><![CDATA[
<?php
	include("UpdateConfig.php");
	echo($g_description);
?>	
	]]></description>
	<items>
<?php
	header('Content-Type: text/xml; charset=utf-8');

	function scan_items($path, $target_path)
	{
		$dir = opendir($path);
		if ($dir)
		{
			while (($file = readdir($dir)) !== false)
			{
				if ($file == "." || $file == "..")
					continue;
					
				if (!is_dir($path."/".$file))
				{
					if ($file[0] == ".")
						continue;
					
					$base_url = $_SERVER['HTTP_HOST'].substr($_SERVER['REQUEST_URI'], 0, -11);
					
					echo("\t\t<item type=\"traktor.update.Resource\">\n");
					echo("\t\t\t<size>".filesize($path."/".$file)."</size>\n");
					echo("\t\t\t<md5>".md5_file($path."/".$file)."</md5>\n");
					echo("\t\t\t<url>http://$base_url/".str_replace(" ", "%20", $path."/".$file)."</url>\n");
					echo("\t\t\t<targetPath>".$target_path."/".$file."</targetPath>\n");
					echo("\t\t</item>\n");
				}
				else
					scan_items($path."/".$file, $target_path."/".$file);
			}
			closedir($dir);
		}
	}
	
	scan_items($g_bundleSourcePath, $g_targetBasePath);

?>
	</items>
	<postActions/>
</object>