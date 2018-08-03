const fs = require('fs')

function emit() {
  let msg = JSON.stringify({ a: Math.random(), b: Math.random() })
  fs.writeFileSync('pub/random', msg)
  setTimeout(emit, 1000)
}

emit()
