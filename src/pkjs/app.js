var myAPIKey = 'eaceb3c6dcff48730ddad91fe09d8f4f';

function iconFromWeatherId(weatherId) {
  if (weatherId < 600) {
    return 2;
  } else if (weatherId < 700) {
    return 3;
  } else if (weatherId > 800) {
    return 1;
  } else {
    return 0;
  }
}

function fetchWeather(latitude, longitude) {
  var req = new XMLHttpRequest();
  req.open('GET', 'http://api.openweathermap.org/data/2.5/weather?' +
    'lat=' + latitude + '&lon=' + longitude + '&cnt=1&appid=' + myAPIKey, true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
		console.log("fetchweather response");
        console.log(req.responseText);
        var response = JSON.parse(req.responseText);
        var temperature = Math.round(response.main.temp - 273.15);
        var icon = iconFromWeatherId(response.weather[0].id);
        var city = response.name;
        console.log(temperature);
        console.log(icon);
        console.log(city);
        Pebble.sendAppMessage({
		  'WEATHER_ICON_KEY': icon,
          'WEATHER_TEMPERATURE_KEY': temperature + '\xB0C',
          'WEATHER_CITY_KEY': city,
        });
      } else {
        console.log('Error');
      }
    }
  };
  req.send(null);
}

function fetchStock() {
  var req = new XMLHttpRequest();
  req.open('GET', 'https://www.google.com/finance/info?q=NYSE:AMD', true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
		console.log("fetchStock response");
//        console.log(req.responseText);
		var fix_response = req.responseText;
		fix_response = fix_response.replace('// ', '');
		fix_response = fix_response.replace('[', '');
		fix_response = fix_response.replace(']', '');
		fix_response = fix_response.replace(/\r?\n|\r/g, "");
		console.log(fix_response);
        var response = JSON.parse(fix_response);  
		var stock = response.l_cur;
		console.log(stock);
        Pebble.sendAppMessage({
		  'STOCK_PRICE_KEY': "$" + stock,
        });
      } else {
        console.log('Error');
      }
    }
  };
  req.send(null);
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
  fetchWeather(coordinates.latitude, coordinates.longitude);
  fetchStock();	
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    'WEATHER_CITY_KEY': 'Loc Unavailable',
    'WEATHER_TEMPERATURE_KEY': 'N/A'
  });
}

var locationOptions = {
  'timeout': 15000,
  'maximumAge': 60000
};

Pebble.addEventListener('ready', function (e) {
  console.log('connect!' + e.ready);
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError,
    locationOptions);
  console.log(e.type);
});

Pebble.addEventListener('appmessage', function (e) {
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError,
    locationOptions);
  console.log(e.type);
  console.log(e.payload.temperature);
  console.log('message!');
});

Pebble.addEventListener('webviewclosed', function (e) {
  console.log('webview closed');
  console.log(e.type);
  console.log(e.response);
});
