
all: main.css main.js

main.css: main.scss
	scss -t compressed --sourcemap $<:$@

main.js: main.coffee
	coffee -cm $<
	uglifyjs --in-source-map $@.map --source-map $@.map -o $@ $@

clean:
	rm -rf main.css main.js main.*.map .sass-cache
