
all: main.css main.js

main.css: main.scss
	scss -t compressed --sourcemap $<:$@

main.js: main.coffee
	coffee -cm $<

clean:
	rm -rf main.css main.js main.*.map .sass-cache
