all: demo.css demo.js

demo.css: demo.scss
	scss $<:$@

demo.js: demo.coffee
	coffee -c $<

clean:
	rm demo.css demo.js
