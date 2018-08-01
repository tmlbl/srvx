const fs = require('fs')

while (true) {
    console.log('Waiting for a message...')
    const msg = fs.readFileSync('pub/foo:bar')
    console.log(`Received message: ${msg}`)
}
