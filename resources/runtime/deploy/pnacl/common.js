// common.js
		
function moduleDidLoad()
{
	var app = document.getElementById('app');
	app.postMessage('create'); 
}

document.addEventListener('DOMContentLoaded', function() {

	var app = document.getElementById('app');
	app.addEventListener('load', moduleDidLoad, true);

});
