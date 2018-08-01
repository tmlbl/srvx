const fs = require('fs')

while (true) {
    console.log('Waiting for a message...')
    const msg = fs.readFileSync('sub/foo:bar')
    console.log(`Received message: ${msg}`)
}
