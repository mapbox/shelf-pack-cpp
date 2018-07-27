## Release Checklist

#### Update version, docs, tag, and publish

Assumes documentation.js v4 has been installed globally:
`npm install -g documentation`

- [ ] git checkout master
- [ ] Update CHANGELOG
- [ ] Update version number in `include/mapbox/shelf-pack.hpp` and `makedocs.sh`
- [ ] ./makedocs.sh
- [ ] git add .
- [ ] git commit -m 'vA.B.C'
- [ ] git tag vA.B.C
- [ ] git push origin master vA.B.C
- [ ] open pull request in `mason` project
