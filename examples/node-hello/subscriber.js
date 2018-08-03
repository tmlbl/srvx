const fs = require('fs')

while (true) {
  const msg = JSON.parse(fs.readFileSync('pub/random'))
  const product = msg.a * msg.b
  fs.writeFileSync('pub/product', JSON.stringify({ product }))
}
