(function(){var Demo,Dots;new(Demo=function(){Demo.prototype.rootId="dots";Demo.prototype.websocketUrl="wss://dotbot.herokuapp.com";Demo.prototype.websocketProtocol="dotbot-stream";Demo.prototype.initialDelay=500;Demo.prototype.moveRequestInterval=500;Demo.prototype.targetQueueSize=20;Demo.prototype.pollingInterval=50;Demo.prototype.colors={R:"red",Y:"yellow",G:"green",B:"blue",V:"violet"};function Demo(){this.root=document.getElementById(this.rootId);this.queue=[];setTimeout(function(_this){return function(){_this.ws=new WebSocket(_this.websocketUrl,_this.websocketProtocol);_this.ws.onopen=_this.onopen.bind(_this);_this.ws.onclose=_this.onclose.bind(_this);return _this.ws.onmessage=_this.onmessage.bind(_this)}}(this),this.initialDelay)}Demo.prototype.onopen=function(){this.running=true;return this.updateIntervalId=setInterval(function(_this){return function(){if(_this.running&&_this.queue.length<_this.targetQueueSize){return _this.ws.send("next")}}}(this),this.moveRequestInterval)};Demo.prototype.onclose=function(){this.running=false;clearInterval(this.updateIntervalId);return console.log("socket closed")};Demo.prototype.onmessage=function(msg){var c,data,row;data=JSON.parse(msg.data);data.grid=function(){var _i,_len,_ref,_results;_ref=data.grid;_results=[];for(_i=0,_len=_ref.length;_i<_len;_i++){row=_ref[_i];_results.push(function(){var _j,_len1,_ref1,_results1;_ref1=row.split("");_results1=[];for(_j=0,_len1=_ref1.length;_j<_len1;_j++){c=_ref1[_j];_results1.push(this.colors[c])}return _results1}.call(this))}return _results}.call(this);if(data.path==null){return this.dots=new Dots(this.root,data.grid,this.update.bind(this))}else{return this.queue.push(data)}};Demo.prototype.update=function(){var data;if((data=this.queue.shift())!=null){return this.dots.drawPath(data.path,data.grid,this.update.bind(this))}else if(this.running){return setTimeout(this.update.bind(this),this.pollingInterval)}};return Demo}());Dots=function(){Dots.prototype.fallingDotsDuration=250;Dots.prototype.drawPathDelay=100;Dots.prototype.clearPathDelay=500;Dots.prototype.dropDotsDelay=150;function Dots(root,grid,next){var r;this.root=root;this.grid=grid;this.dots=function(){var _i,_results;_results=[];for(r=_i=0;_i<=5;r=++_i){_results.push([])}return _results}();this.dropDots(this.grid,next)}Dots.prototype.dropDots=function(grid,next){var c,dot,nextRowToFill,r,_i,_j,_k;this.grid=grid;for(c=_i=0;_i<=5;c=++_i){nextRowToFill=5;for(r=_j=5;_j>=0;r=--_j){if((dot=this.getDot(r,c))!=null){if(r!==nextRowToFill){this.setDot(nextRowToFill,c,dot);this.delDot(r,c);dot.dataset.row=nextRowToFill}nextRowToFill--}}if(nextRowToFill>=0){for(r=_k=0;0<=nextRowToFill?_k<=nextRowToFill:_k>=nextRowToFill;r=0<=nextRowToFill?++_k:--_k){this.newDot(r,c)}}}return setTimeout(next,this.fallingDotsDuration+this.drawPathDelay)};Dots.prototype.newDot=function(r,c){var dot;dot=document.createElement("div");dot.className="dot";dot.dataset.row=r;dot.dataset.col=c;dot.dataset.color=this.grid[r][c];this.root.appendChild(dot);return this.setDot(r,c,dot)};Dots.prototype.getDot=function(r,c){return this.dots[r][c]};Dots.prototype.setDot=function(r,c,dot){return this.dots[r][c]=dot};Dots.prototype.delDot=function(r,c){return delete this.dots[r][c]};Dots.prototype.drawPath=function(path,newGrid,next){var drawNextSegment;return(drawNextSegment=function(_this){return function(path){var c,r,segment,_ref;_ref=path[0],r=_ref[0],c=_ref[1];_this.selectDot(_this.getDot.apply(_this,path[0]));if(path.length>1){segment=_this.newPathSegment(path[0],path[1]);return segment.addEventListener("animationend",function(){return drawNextSegment(path.slice(1))})}else if(path.length===1){return setTimeout(_this.clearPath.bind(_this,newGrid,next),_this.clearPathDelay)}}}(this))(path)};Dots.prototype.selectDot=function(dot,checkForCycle){var anim;if(checkForCycle==null){checkForCycle=true}if(checkForCycle&&dot.classList.contains("marked-for-deletion")){setTimeout(function(_this){return function(){var c,color,r,_i,_results;color=dot.dataset.color;_this.root.dataset.color=color;_results=[];for(r=_i=0;_i<=5;r=++_i){_results.push(function(){var _j,_results1;_results1=[];for(c=_j=0;_j<=5;c=++_j){if((dot=this.getDot(r,c)).dataset.color===color){_results1.push(this.selectDot(dot,false))}else{_results1.push(void 0)}}return _results1}.call(_this))}return _results}}(this));return}dot.classList.add("marked-for-deletion");anim=document.createElement("div");anim.className="dot selecting";anim.dataset.row=dot.dataset.row;anim.dataset.col=dot.dataset.col;anim.dataset.color=dot.dataset.color;anim.addEventListener("animationend",function(){return this.parentNode.removeChild(this)});return this.root.appendChild(anim)};Dots.prototype.newPathSegment=function(_arg,_arg1){var c1,c2,r1,r2,segment;r1=_arg[0],c1=_arg[1];r2=_arg1[0],c2=_arg1[1];segment=document.createElement("div");segment.className="path-segment from-"+r1+"-"+c1+"-to-"+r2+"-"+c2;segment.dataset.color=this.grid[r1][c1];this.root.appendChild(segment);return segment};Dots.prototype.clearPath=function(newGrid,next){var dot,segments,selectedDots,_i,_len;if(this.root.dataset.color!=null){delete this.root.dataset.color}segments=this.root.getElementsByClassName("path-segment");while(segments[0]){segments[0].parentNode.removeChild(segments[0])}selectedDots=this.root.getElementsByClassName("dot marked-for-deletion");for(_i=0,_len=selectedDots.length;_i<_len;_i++){dot=selectedDots[_i];this.delDot(dot.dataset.row,dot.dataset.col);dot.classList.add("shrinking");dot.addEventListener("animationend",function(){return this.parentNode.removeChild(this)})}return setTimeout(this.dropDots.bind(this,newGrid,next),this.dropDotsDelay)};return Dots}()}).call(this);
//# sourceMappingURL=main.js.map