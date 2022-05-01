//
// Cote UI de l'application "lucioles"
//
// Auteur : G.MENEZ
// RMQ : Manipulation naive (debutant) de Javascript
//

const MAC_ADDRESS_ESP = [
    {
        name: 'Alexandre', mac_address:'80:7D:3A:FD:C7:1C'
    },
    {
        name: 'Youssef', mac_address:'30:AE:A4:8B:44:EC'
    },
    {
        name: 'vador', mac_address:'80:7D:3A:FD:E8:E8'
    }

]

let val;
function init() {
    
    //=== Initialisation des traces/charts de la page html ===
    // Apply time settings globally
    Highcharts.setOptions({
	global: { // https://stackoverflow.com/questions/13077518/highstock-chart-offsets-dates-for-no-reason
            useUTC: false,
            type: 'spline'
	},
	time: {timezone: 'Europe/Paris'}
    });
    // cf https://jsfiddle.net/gh/get/library/pure/highcharts/highcharts/tree/master/samples/highcharts/demo/spline-irregular-time/
    chart1 = new Highcharts.Chart({
        title: {text: 'Temperatures'},
	subtitle: { text: 'Irregular time data in Highcharts JS'},
        legend: {enabled: true},
        credits: false,
        chart: {renderTo: 'container1'},
        xAxis: {title: {text: 'Heure'}, type: 'datetime'},
        yAxis: {title: {text: 'Temperature (Deg C)'}},
        series: MAC_ADDRESS_ESP.map(({ name }) => ({ name, data: [] })),
	//colors: ['#6CF', '#39F', '#06C', '#036', '#000'],
	colors: ['red', 'green', 'blue'], 
        plotOptions: {line: {dataLabels: {enabled: true},
			     //color: "red",
			     enableMouseTracking: true
			    }
		     }
    });
    chart2 = new Highcharts.Chart({
        title: { text: 'Lights'},
        legend: {title: {text: 'Lights'}, enabled: true},
        credits: false,
        chart: {renderTo: 'container2'},
        xAxis: {title: {text: 'Heure'},type: 'datetime'},
        yAxis: {title: {text: 'Lumen (Lum)'}},
        series: MAC_ADDRESS_ESP.map(({ name }) => ({ name, data: [] })),
	//colors: ['#6CF', '#39F', '#06C', '#036', '#000'],
	colors: ['red', 'green', 'blue'], 
        plotOptions: {line: {dataLabels: {enabled: true},
			     enableMouseTracking: true
			    }
		     }
    });


    //=== Gestion de la flotte d'ESP =================================
    var which_esps = MAC_ADDRESS_ESP.map(({ mac_address }) => mac_address);
    
    for (var i = 0; i < which_esps.length; i++) {
	console.log('process_esp : ', i)
    process_esp(which_esps, i)
    }

    createMap()
};

function createMap() {
    var villes = {
        "Alexandre": { "lat": 45.00, "lon": 8.73 },
        "Youssef": { "lat": 43.591319, "lon": 7.097996 },
        "vador": { "lat": 41.403458, "lon": 2.174632 }, 
    };

    // Créer l'objet "macarte" et l'insèrer dans l'élément HTML qui a l'ID "map"
    var lat = 48.852969;
    var lon = 50.349903;
    // Créer l'objet "macarte" et l'insèrer dans l'élément HTML qui a l'ID "map"
    macarte = L.map('map').setView([lat, lon], 3);
    // Leaflet ne récupère pas les cartes (tiles) sur un serveur par défaut. Nous devons lui préciser où nous souhaitons les récupérer. Ici, openstreetmap.fr
    L.tileLayer('https://{s}.tile.openstreetmap.fr/osmfr/{z}/{x}/{y}.png', {
        // Il est toujours bien de laisser le lien vers la source des données
        attribution: 'données © OpenStreetMap/ODbL - rendu OSM France',
        minZoom: 1,
        maxZoom: 20
    }).addTo(macarte);
    // Nous parcourons la liste des villes
    for (ville in villes) {
        var marker = L.marker([villes[ville].lat, villes[ville].lon]).addTo(macarte);
        marker.bindPopup(ville);
    }
    
}

//=== Installation de la periodicite des requetes GET============
function process_esp(which_esps,i){
    const refreshT = 10000 // Refresh period for chart
    esp = which_esps[i];    // L'ESP "a dessiner"
    console.log('process_esp : ', esp) // cf console du navigateur
    
    // Gestion de la temperature
    // premier appel pour eviter de devoir attendre RefreshT
    get_samples('/esp/projecttemperature', chart1.series[i], esp);
    //calls a function or evaluates an expression at specified
    //intervals (in milliseconds).
    window.setInterval(get_samples,
		       refreshT,
		       '/esp/projecttemperature',     // param 1 for get_samples()
		       chart1.series[i],// param 2 for get_samples()
		       esp);            // param 3 for get_samples()

    // Gestion de la lumiere
    get_samples('/esp/projectlight', chart2.series[i], esp);
    window.setInterval(get_samples,
		       refreshT,
		       '/esp/projectlight',     // URL to GET
		       chart2.series[i], // Serie to fill
		       esp);             // ESP targeted
}


//=== Recuperation dans le Node JS server des samples de l'ESP et 
//=== Alimentation des charts ====================================
function get_samples(path_on_node, serie, wh){
    // path_on_node => help to compose url to get on Js node
    // serie => for choosing chart/serie on the page
    // wh => which esp do we want to query data

    console.log('get samples !');
    //node_url = window.location.href;
    node_url = 'https://iot21804934m1.herokuapp.com'
    //node_url = 'http://localhost:3000'
    //node_url = 'http://134.59.131.45:3000'
    //node_url = 'http://192.168.1.101:3000'

    //https://openclassrooms.com/fr/courses/1567926-un-site-web-dynamique-avec-jquery/1569648-le-fonctionnement-de-ajax
    $.ajax({
        url: node_url.concat(path_on_node), // URL to "GET" : /esp/temp ou /esp/light
        type: 'GET',
        headers: { Accept: "application/json", },
	data: {"who": wh}, // parameter of the GET request
        success: function (resultat, statut) { // Anonymous function on success
            let listeData = [];
            resultat.forEach(function (element) {
		listeData.push([Date.parse(element.date),element.value]);
		//listeData.push([Date.now(),element.value]);
            });
            serie.setData(listeData); //serie.redraw();
        },
        error: function (resultat, statut, erreur) {
        },
        complete: function (resultat, statut) {
        }
    });
}


//assigns the onload event to the function init.
//=> When the onload event fires, the init function will be run. 
window.onload = init;


