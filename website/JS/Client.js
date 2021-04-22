var user = null;

//Checks user has entered correct username and password
//If so login, else show error message
function login() {
	var xmlHttp = new XMLHttpRequest();
	user = document.getElementById("logname").value;
	var pass = document.getElementById("logpass").value;
	console.log(user);
	xmlHttp.open( "POST", "/login/", true ); // false for synchronous request
	xmlHttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	xmlHttp.onload = function() {
        if (xmlHttp.status == 200) {
			sessionStorage.setItem('username', user);
			//alert(sessionStorage.getItem('username'));
            // Typical action to be performed when the document is ready:
			window.location = "http://localhost:9432/News.html";
        } else {
            document.getElementById("ErrorLogin").innerHTML = "<p class = \"login-err\">Incorrect Username or Password</p>";
		}
    };
    xmlHttp.send("Username=" + user + "&Password=" + pass);
}

//Check that the user has not picked an already used username
//If already used, show error message
//Else register and login
function register() {
	user = document.getElementById("regname").value;
	var pass = document.getElementById("regpass").value;
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open( "POST", "/reg/", true ); // false for synchronous request
    xmlHttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xmlHttp.onload = function() {
		if(xmlHttp.status == 401){
			document.getElementById("Error").innerHTML = "<p class = \"login-err\">Username already in use</p>";
		} else {
			sessionStorage.setItem('username', user);
			window.location = "http://localhost:9432/News.html";
		}
	};
    xmlHttp.send("Username=" + user + "&Password=" + pass);


}

//Get and display the news from Gaurdian API keeping in mind the user's filters
function httpGet()
{
	user = sessionStorage.getItem('username');
	var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", "/News/" + user, true ); // false for synchronous request
    xmlHttp.onreadystatechange = function() {
		console.log(this.readyState + ", " + this.status);
        if (this.readyState == 4 && this.status == 200) {
            // Typical action to be performed when the document is ready:
            data = xmlHttp.responseText;
            document.getElementById("news").innerHTML = "<p class = \"list-content\">" + data + "</p>";
           toMicro(data);
        } else {
		}
    };
    xmlHttp.send( null );
}

//Get and display the user's filters from the database 
function httpGetFilt()
{
    var xmlHttp = new XMLHttpRequest();
    var user = sessionStorage.getItem('username');
    xmlHttp.open( "GET", "/NewsFilters/" + user, true ); // false for synchronous request
    xmlHttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            // Typical action to be performed when the document is ready:
            data = xmlHttp.responseText;
            document.getElementById("Filters").innerHTML =  "<p class = \"list-content\">" + data + "</p>";
        }
    };
    xmlHttp.send( null );
}

//Adds a filter to the database and refreshes the displayed list
function httpPost()
{
    var xmlHttp = new XMLHttpRequest();
	var user = sessionStorage.getItem('username');
	console.log(user);
    var filter = document.getElementById("filter").value;
    xmlHttp.open( "POST", "/AddFilter/", true ); // false for synchronous request
    xmlHttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xmlHttp.onload = function() {
    };
	xmlHttp.send("Username=" + user + "&Filter=" + filter);
	httpGetFilt();
}

//Deletes a filterfrom the database and refreshes the displayed list
function httpPostDel()
{
    var xmlHttp = new XMLHttpRequest();
    var user = sessionStorage.getItem('username');
    var filter = document.getElementById("filter").value;
    xmlHttp.open( "POST", "/DeleteFilter/", true ); // false for synchronous request
    xmlHttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xmlHttp.onload = function() {
    };
	xmlHttp.send("Username=" + user + "&Filter=" + filter);
	httpGetFilt();
}

//Send data to the microbit (might be redundant)
function toMicro(data){
    $.ajax({
        type : "GET",
        url:'http://127.0.0.1:5000',
        dataType : "jsonp",
        data : data,
        crossDomain : true,
        success : function(result) {
          jQuery("#clash").html(result); 
        },error : function(result){
           console.log(result);
       }
      });
}

function test(data){
    alert("In " + JSON.stringify(data.name) + " it is " + JSON.stringify(data.weather[0].main) + " and the temperature is " +  JSON.stringify((data.main.temp - 273).toFixed(2)) + " Degrees C.");
}

//request weather from the weather api
function getWeather()
{
    $.ajax({
        type : "GET",
        url:"http://api.openweathermap.org/data/2.5/weather?q=Glasgow&appid=b58beed2d95cd833dd6daa5dd2034fc2&callback=test",
        dataType : "jsonp",
        crossDomain : true,
        success : function(result) {
          jQuery("#clash").html(result); 
        },error : function(result){
           console.log(result);
       }
      });
}