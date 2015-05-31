
all: demo.css demo.js

demo.css: demo.scss
	scss -t compressed --sourcemap $<:$@

demo.js: demo.coffee
	coffee -cm $<

clean:
	rm -f demo.css demo.js demo.*.map
