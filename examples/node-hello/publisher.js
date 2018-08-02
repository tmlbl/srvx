const fs = require('fs')

function publish() {
    console.log('Publishing a message...')
    fs.writeFileSync('pub/foo:bar', JSON.stringify({
        microservices: true,
        time: new Date()
    }))
    setTimeout(publish, 1000)
}

publish()
