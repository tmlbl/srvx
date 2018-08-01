const fs = require('fs')

function publish() {
    console.log('Publishing a message...')
    fs.writeFileSync('pub/foo:bar', 'hello')
    setTimeout(publish, 1000)
}

publish()
