//Access via http://localhost:9432/Login.html

var http = require('http');
var request = require('request');
var url = require('url');
let express = require('express');
let app = express();
let bodyParser = require("body-parser");
let path = require('path');
let mysql = require('mysql');

var con = mysql.createConnection({  //Connect to db with these details.
  host: "tk58.host.cs.st-andrews.ac.uk",
  user: "tk58",
  password: "7WCZR.99yjhg1X",
  database: "tk58_microbit_users"
});

con.connect( function (err) {		//On error connecting to db
  if (err) {
      console.log('Error connecting to Db');
      return;
  }
  console.log('Connection established');
});

app.use(bodyParser());

app.use(express.static(path.join(__dirname, 'website')));		//For providing webpages.


request('https://content.guardianapis.com/search?api-key=aec73380-c397-4e0b-8df0-030861b834be', { json: true }, (err, res, body) => {	//Get guardian news.
  if (err) { return console.log(err); }
  articles = body.response.results;	//Loop over news.
  
  for(var i = 0; i < articles.length; i++) {
    var obj = articles[i];

    console.log(obj.webTitle);
	}
});

var port = 9432;		//Run on this port

//On receiving news request.
app.get('/News/:username', function (req, res) {
  res.writeHead(200);
  var filters = [];
  con.query('SELECT Filter FROM Filters where ? = Filters.Username', [req.params.username], function (err, rows) {		//Obtain filters for given user.
    if (err) throw err;
    console.log('Data received from DB');
    for (var i = 0; i < rows.length; i++) {
      filters.push(rows[i].Filter);
      console.log(rows[i].Filter);
    };
    for(var i = 0; i < articles.length; i++) {
      var filtered = false;
      filters.forEach(element => {
        if(articles[i].webTitle.toLowerCase().includes(element.toLowerCase())){
          filtered = true;
        }
      });
      var data = (articles[i].webTitle).concat("<br>");
	  if(!filtered)res.write(data);
  }
  res.end();	
  }); 
});

//Return all filters for a given user.
app.get("/NewsFilters/:username", function (req, res) {
  res.writeHead(200);
  con.query('SELECT Filter FROM Filters where ? = Filters.Username', [req.params.username], function (err, rows) {
    if (err) throw err;
    console.log('Data received from DB');
    for (var i = 0; i < rows.length; i++) {
      res.write(rows[i].Filter + "<br>");
    };
    res.end();
  });
});

//Remove filter from selected user's account.
app.post('/DeleteFilter/', function (req, res){
  var username = req.body.Username;
  var filter = req.body.Filter;
  console.log(username);
  console.log(filter);
  con.query("Delete from Filters where Username = ? and Filter = ?", [username, filter], function (err, result) {
    console.log("1 record Removed");
  })
  res.end("Test");
});

//Add filter request, add filter to selected user's account
app.post('/AddFilter/', function (req, res){
  var username = req.body.Username;
  var filter = req.body.Filter;
  console.log(username);
  console.log(filter);
  con.query("Insert into Filters Values (?, ?)", [username, filter], function (err, result) {
    console.log("1 record inserted");
  })
  res.end("Test");
});

//Upon receiving Time request, get time at loc of server.
app.get('/Time', function(req, res){
  res.writeHead(200);
  var d = new Date;
  res.write(d.getHours().toString() + ":" + d.getMinutes().toString() + ":" + d.getSeconds().toString());
  res.end();
})

//Upon receiving news request, return weather from API.
app.get('/Weather', function (req, res) {
	request('http://api.openweathermap.org/data/2.5/weather?q=Glasgow&appid=b58beed2d95cd833dd6daa5dd2034fc2', { json: true }, (err, resp, body) => {
		var data = body;
		 res.write("In " + JSON.stringify(data.name) + " it is " + JSON.stringify(data.weather[0].main) + " and " +  JSON.stringify((data.main.temp - 273).toFixed(2)) + " Degrees C.")
		 res.end(); 
	}); 
});

//Upom receiving a login request
app.post('/login', function(req, res) {
	console.log("Login request");
	console.log(req.body.Username);	//Check user exists if not return error.
	console.log(req.body.Password);
	con.query("select * from users where username = ? AND password = ?;", [req.body.Username, req.body.Password], function(err, rows) {
		console.log(rows.length);
		if(rows.length == 0){
			console.log("User Not Found");
			res.writeHead(401);
			res.end();	
		} else {
			console.log(req.body.Username + " Logged In.")
			res.writeHead(200);
			res.end();
		}
	})
	
});

//On registering a new account
app.post('/reg', function(req, res) {
	var username = req.body.Username;	//Get username and pass from body
	var password = req.body.Password;
	con.query("Insert into users Values (?, ?)", [username, password], function (err, result) {	//Check account exists.
		if (err){
			console.log("User already exists");	//If: thow error
			res.writeHead(401);
			res.end();	

		} else {
			console.log("1 record inserted");	//If not: add account
			console.log("Added User: " + username + " with " + password);
			res.writeHead(200);
			res.end();
		}
	})
	

});

//For twitter - not supported.
app.get('/Twitter', function (req, res) {
  res.writeHead(200);
  res.write("Under Construction");  
  res.end();	
});
app.listen(port);