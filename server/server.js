// Create a server with express
const express = require('express')
const app = express()
var cors = require('cors')
// Set the port of the server to 3000
const port = 3000

// Parse the body of the request to JavaScript object, so server can use it
app.use(express.json());
app.use(cors());

// app.use(express.static('public'));

let temp = 0;
let hum = 0;


// // When opening the URL /, the server will respond with "Hello World!"
// app.get('/', (req, res) => {
//   res.sendFile(__dirname + '/public/client.html')
// })

// When posting to the URL /, print the body of the request and respond with "1"
app.post('/', (req, res) => {
  console.log('Full request body:', req.body);  // Debug
  console.log('Temperature:', req.body.temperature);
  console.log('Humidity:', req.body.humidity);

  temp = req.body.temperature;
  hum = req.body.humidity;

  if (temp === undefined || hum === undefined) {
        console.log('ERROR: Data is undefined!');
        return res.status(400).send('Missing data');
    }
    
  // Xử lý data ở đây...
  res.send('Data received successfully');
})

app.get('/data', (req, res) => {
  let mData = {mTem: temp, mHum: hum};
  res.json(mData);
})

// Server starts listening on port 3000, then prints "Example app listening on port 3000"
app.listen(port, () => {
  console.log(`Example app listening on port ${port}`)
})