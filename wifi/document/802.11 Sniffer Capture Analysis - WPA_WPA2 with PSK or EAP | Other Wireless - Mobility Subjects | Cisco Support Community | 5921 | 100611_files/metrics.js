/**
 * metrics.min.js
 * $Revision: $
 *
 *  metrics.min.js -- aggregated and minified
 *  DO NOT EDIT THIS FILE -- edit the individual files and rebuild
 *
 * supports metrics on non-vendor kit sites  (c) 1992-2011 Cisco Systems, Inc. All rights reserved.
 *   Terms and Conditions: http://cisco.com/en/US/swassets/sw293/sitewide_important_notices.html
 *  (minified)   (c) 1992-2011 Cisco Systems, Inc. All rights reserved.
 *   Terms and Conditions: http://cisco.com/en/US/swassets/sw293/sitewide_important_notices.html
 * 
 *
 * MANIFEST:
 *      /cdc/fw/j/ext_util_core.js 1.10 
 *      /cdc/fw/m/ext_rules-lib.js 1.4 
 *      /cdc/fw/m/ext_liveManager.js 1.19 
 */

/*!
 * BEGIN /cdc/fw/j/ext_util_core.js
 */
if(window.cdc===undefined){window.cdc={}}cdc.util={ensureNamespace:function(d){if(!d){return}var f=d.split(".");var g=window;for(var c=0;c<f.length;c++){var h=f[c];if(typeof(g[h])!="object"){g[h]={}}g=g[h]}},checkClear:function(c,d){if(c.value==d){c.value=""}if(c.id=="searchPhrase"){if(!document.getElementById("search-drop-down")){setupSearch()}if(document.getElementById("search-drop-down")){showSuggestionsContainer()}}},cacheBust:function(d,g){if(!g){g="cacheReset"}var f="?";if(d.match(/(ng-prod1|\?)/)){f="&"}var c=f+g+"=";return d+c+cdc.util.randomNumber()},randomNumber:function(c){if(!c){c=1000}var f=Math.floor(Math.random()*c)+1;var d=(new Date).getTime();var g=d+"-"+f;return g},isAuthenticated:function(){var c=cdc.cookie.getCookie("SMSESSION");var d=null;if(c&&c!=""&&c!="LOGGEDOFF"){d=true}else{d=false}return d},authStatus:"unready",checkLoginQueue:[],checkLogin:function(c){if(typeof c!="function"){if(typeof console!="undefined"&&console.trace){console.log("cdc.util.checkLogin: expecting a function, got a "+typeof c);console.trace()}return}if(cdc.util.authStatus!="unready"){c(cdc.util.authStatus)}else{cdc.util.checkLoginQueue.push(c)}},notifyLoginQueue:function(c){if(cdc.util.authStatus=="unready"){for(i=0;i<cdc.util.checkLoginQueue.length;i++){cdc.util.checkLoginQueue[i](c)}}cdc.util.authStatus=c},openCdcPopup:function(g,h,c){if(!g){return true}var j="";var k="";var f="globalCDCpopup";if(typeof(g)=="object"){h=g.width;c=g.height;xtop=typeof(g.top)!="undefined"?g.top:"";left=typeof(g.left)!="undefined"?g.left:"";k="top="+xtop+",left="+left+",";if(typeof(g.windowName)!="undefined"){f=g.windowName}if(g.controls!=false){if(typeof(g.location)!="undefined"&&g.location=="no"){var j=",toolbar=yes,location=no,menubar=yes"}else{var j=",toolbar=yes,location=yes,menubar=yes"}}g=g.address}h=isNaN(parseInt(h))?550:parseInt(h);c=isNaN(parseInt(c))?550:parseInt(c);if(document.all){h=h+20}k+="width="+h+",height="+c+",status=yes,scrollbars=yes,resizable=yes"+j;var d=window.open(g,f,k);if(d){d.focus()}return false},getParameter:function(h,f){var c;if(f){c=f.slice(f.indexOf("?")+1)}c=(c)?c:window.location.search;var g="";var j=c.indexOf(h);if(j!=-1){j+=h.length+1;var d=c.indexOf("&",j);if(d==-1){d=c.length}g=c.substring(j,d)}return g},onElementLoadById:(function(){var c=false;jQuery(document).ready(function(){c=true});function d(f){if(c){return true}while(f){if(f.nextSibling){return true}f=f.parentNode}return false}return function(h,g){var f=false;(function(){f=f||document.getElementById(h);if(f&&d(f)){g.call(f)}else{window.setTimeout(arguments.callee,100)}})()}})(),logoutdialog:{show:function(){cdc.util.ensureNamespace("cdc.local.wpx");if(!jQuery("#logoutmsg").length){cdc.local.wpx=jQuery.extend({LOGOUT_MODAL_TITLE:"Log Out",LOGOUT_MODAL_QUERY:"You are about to log out of Cisco.com.<br />If your task is incomplete, please click Cancel to finish or save.",LOGOUT_YES_BUTTON_TEXT:"Log Out",LOGOUT_NO_BUTTON_TEXT:"Cancel"},cdc.local.wpx);cdc.util.logoutdialog.url=this.href;cdc.util.logoutdialog.html='<div id="logoutmsg"><span id="lm-corner-top"><span></span></span><h4>'+cdc.local.wpx.LOGOUT_MODAL_TITLE+"</h4><div>"+cdc.local.wpx.LOGOUT_MODAL_QUERY+'</div><a class="a00v1" id="logoutbtn" href="'+cdc.util.logoutdialog.url+'">'+cdc.local.wpx.LOGOUT_YES_BUTTON_TEXT+'</a><a id="logoutclose" class="a00v1" href="javascript:return false">'+cdc.local.wpx.LOGOUT_NO_BUTTON_TEXT+'</a><span id="lm-corner-bot"><span></span></span></div>';jQuery(this).append(cdc.util.logoutdialog.html);jQuery("#logoutmsg").jqm({modal:true,toTop:true}).jqmAddClose("#logoutmsg #logoutclose")}jQuery("#logoutmsg").css("left",jQuery("#fw-banner").offset().left+240);jQuery("#logoutmsg").jqmShow();setTimeout(function(){jQuery("#logoutbtn").focus()},0);jQuery("#logoutclose").keydown(function(c){if(c.keyCode==9){jQuery("#logoutbtn").focus();c.preventDefault()}});jQuery("#logoutbtn").keydown(function(c){if(c.keyCode==9&&c.shiftKey==1){jQuery("#logoutclose").focus();c.preventDefault()}});return false}},ls:{setConfigInfo:function(c,f){try{localStorage[c]=JSON.stringify(f);return true}catch(d){return false}},getConfigInfo:function(c){try{return JSON.parse(localStorage[c])}catch(d){return false}},deleteConfigInfo:function(c){try{localStorage.removeItem(c);return true}catch(d){return false}}},ut:{setConfigInfo:function(c){cdc.util.ls.setConfigInfo("utConfig",c)},getConfigInfo:function(){var d=cdc.util.ls.getConfigInfo("utConfig");var c=new Date();if(d&&d.expiry<c.getTime()){if(console){console.log("Deleting expired utConfig.  Values were:"+JSON.stringify(d)+"You must resave them if you need them.")}cdc.util.ut.deleteConfigInfo();d=false}return d},deleteConfigInfo:function(){cdc.util.ls.deleteConfigInfo("utConfig")}},locale:{languageforTheatercode:{ES:"es_ES",DE:"de_DE",FR:"fr_FR",PL:"pl_PL",BR:"pt_BR",RU:"ru_RU",JP:"ja_JP",KR:"ko_KR",CN:"zh_CN",CZ:"cs_CZ",IT:"it_IT",TH:"th_TH",TR:"tr_TR",VN:"vi_VN",LA:"es_LA"},theatercodeforLanguage:{es:"es_ES",de:"de_DE",fr:"fr_CA",vi:"vi_VN",pl:"pl_PL",pt:"pt_BR",ru:"ru_RU",ja:"ja_JP",ko:"ko_KR",zh:"zh_CN",cs:"cs_CZ",it:"it_IT",th:"th_TH",tr:"tr_TR",en:"en_US"},getLocale:function(k){var n,g,u,o,h="";if(!k||k==null||k==undefined){if(document.referrer){k=document.referrer.toString()}}var q=cdc.util.ls.getConfigInfo("localeinfo");var m=cdc.util.getParameter("loginlocale",k);var p=cdc.util.getParameter("locale",k);var r=cdc.util.getParameter("country",k.toLowerCase());var j=cdc.util.getParameter("language",k.toLowerCase());var t=k.match(/(\/web\/.*[A-Z]{2,3}\/[a-z]{2}\/)/g);var x=k.match(/(\/web\/[A-Z]{2,3}\/.*\_[a-z]{2}[\.\/])/g);var w=k.match(/(\/web\/.*[A-Z]{2,3}\/)/g);var d=k.match(/(\/web\/.*\/global\/.*\_[a-z]{2,3}[\.\/])/g);var l=k.match(/(\/web\/.*\_[a-z]{2,2}[\.\/])/g);var f=k.match(/(\/en\/US\/)/g);if(q){n=q.toString().split("_");g=n[0];u=n[1];o="localstorage";h="localstorage"}else{if((m!="")||(p!="")||(j!="")||(r!="")){if(m||p){n=m.toString().split("_");if(n==""){n=p.toString().split("_")}g=n[0];u=n[1]}else{if(j&&r){g=j;u=r.toUpperCase()}}o="parameter";h="parameter"}else{if(t||x||w||d||l||f){if(t){n=t.toString().split("/");g=n[n.length-2];u=n[n.length-3];o="url";h="url"}else{if(x){n=x.toString().split("_");g=n[n.length-1].replace(/[./]/g,"");theaterArr=x.toString().split("/");u=theaterArr[2];o="url";h="url"}else{if(w){theaterArr=w.toString().split("/");u=theaterArr[theaterArr.length-2];g=false;if(u in cdc.util.locale.languageforTheatercode){g=(cdc.util.locale.languageforTheatercode[u]).toString().split("_")[0];o="url";h="url"}}else{if(d){n=d.toString().split("_");if(n.length>3){g=n[n.length-1].replace(/[./]/g,"");u=n[n.length-2].toUpperCase();h="url";o="url"}else{u=n[n.length-1].replace(/[./]/g,"").toUpperCase();g=false;o=false;h="url"}}else{if(l){u=false;h=false;o="url";n=l.toString().split("_");g=n[n.length-1].replace(/[./]/g,"");if(g in cdc.util.theatercodeforLanguage){u=(cdc.util.theatercodeforLanguage[g]).toString().split("_")[1]}}else{if(f){n=f.toString().split("/");g=n[1];theaterArr=f.toString().split("/");u=theaterArr[2];o="url";h="url"}}}}}}}else{if(window.navigator.language){n=(window.navigator.language).toString().split("-");g=n[0];u=n[1];if(u){u=u.toUpperCase()}if(g in cdc.util.locale.theatercodeforLanguage&&u==undefined){u=(cdc.util.locale.theatercodeforLanguage[g]).toString().split("_")[1]}o="Browser";h="Browser"}else{if(k==location.href&&(!u||!g)){if((u&&!g)||(!u&&g)){var c=document.getElementsByTagName("meta");for(var v in c){if(c[v].name=="language"&&!g){u=c[v].content;o="meta"}if(c[v].name=="country"&&!u){u=c[v].content;h="meta"}}}}else{g=false;u=false;o=false;h=false}}}}}return{getLanguage:g,getCountry:u,getLocale:g+"_"+u,matchCountry:h,matchLanguage:o}}},event:{defer:(function(){var f=0,c=300,d={};return{queueEvent:function(j,g,h){if(h&&typeof(h)==="number"){c=h}if(f&&j.id in d){delete d[j.id]}else{d[j.id]=g}if(f){clearTimeout(f)}f=setTimeout("cdc.util.event.defer.runPendingEvents()",c)},runPendingEvents:function(){for(var h in d){var g=d[h];var j=document.getElementById(h);g(j)}d={}}}})()},findEnvironment:function(){return cdc.util.matchEnvironment(window.location.host)},matchEnvironment:function(c){var j=["www[0-9]*","apps","cdx","cepx-active-prod[0-9]*","wemapp-(author|publish)-(prod[0-9]|nprd)[0-9]*-[0-9]*","www-(author|test|publish)","www-(author|test|publish)-nprd","wwwin-tools","cisco-apps","grs","investor","newsroom","origin-software","papps","software","sso[0-9]*","tools"];var f=["apps-lt","apps-stage","cdx-stage","(cepx|ecmx)-(active|staging|wip)-(lt|stage)[0-9]*","fdk-author-lt","fdk-author-stage","fdk-(publish-)?lt[0-9]*","fdk-(publish-)?.?stage[0-9]*","wemapp-(author|publish)-stage[0-9]*-[0-9]*","papps-stage[0-9]*","software-lt","software-stage","sso-nprd[0-9]*","tools-lt","tools-stage","www-lt[0-9]*","www-(author-|publish-)*stage[0-9]*","wwwin-tools-(dev|stage|lt)"];var k=["apps-dev","cdx-dev","cepx-active-dev[0-9]*","ecmx-active-dev[0-9]*","fdk-(author-)?dev[0-9]*","fdk-(author-)?devint[0-9]*","wemapp-(author|publish)-(dev|devint|idev)[0-9]*-[0-9]*","papps-dev[0-9]*","software-dev","sso-idev[0-9]*","tools-dev","www-(dev|idev)[0-9]*","localhost"];var g=new RegExp("^("+j.join("|")+")(\\.|:|$)");var d=new RegExp("^("+f.join("|")+")(\\.|:|$)");var h=new RegExp("^("+k.join("|")+")(\\.|:|$)");return(function(l){if(g.test(l)){return"prod"}if(d.test(l)){return"stage"}if(h.test(l)){return"dev"}return"unknown"})(c)},js:{extendProperties:function(c){for(prop in this){if(this.hasOwnProperty(prop)){if(prop=="extendProperties"){continue}c[prop]=this[prop]}}return c},nextElemSibling:function(){if(this.nextElementSibling){return cdc.util.js.extendProperties(this.nextElementSibling)}var c=this.nextSibling;while(c&&c.nodeType!=1){c=c.nextSibling}return cdc.util.js.extendProperties(c)},previousElemSibling:function(){if(this.previousElementSibling){return cdc.util.js.extendProperties(this.previousElementSibling)}var c=this.previousSibling;while(c&&c.nodeType!=1){c=c.previousSibling}return cdc.util.js.extendProperties(c)},lastElemChild:function(){if(this.lastElementChild){return cdc.util.js.extendProperties(this.lastElementChild)}var c=this.lastChild;while(c&&c.nodeType!=1){c=c.previousSibling}return cdc.util.js.extendProperties(c)},removeClass:function(c,d){c.className=c.className.replace(new RegExp("(\\s|^)"+d+"(\\s|$)")," ").replace(/^\s+|\s+$/g,"")}}};cdc.util.is1x=(window.location.href.indexOf("/en/US/")>1);cdc.util.testResponsive=function(){var c=new RegExp("fw-res|fw-satellite");return document.body?c.test(document.body.className):false};cdc.util.isResponsive=cdc.util.testResponsive();cdc.mru={serviceHost:"",serviceUrl:"/cisco/web/cdc/psa/mru?command=update&callbackFunctionName=somevalue",timeOutMsecs:250,makeMruRequest:function(f,d){var j=cdc.mru.serviceHost+cdc.mru.serviceUrl;if(d){j+=d}else{if(f.mruExpando){j+=f.mruExpando}else{if(f.rel){j+=f.rel}}}var h=f.href;var g=f.href;if(cdc.debug.on){g+="?timeout";h+="?serviceReturn"}var k=function(){window.location.href=h};cdc.mru.tempDoc=document.createElement("iframe");var c=document.getElementById("framework-footer")?document.getElementById("framework-footer"):(document.getElementById("fw-footer")?document.getElementById("fw-footer"):null);if(c!==null){c.appendChild(cdc.mru.tempDoc)}if(cdc.mru.tempDoc.attachEvent){cdc.mru.tempDoc.attachEvent("onload",k)}else{cdc.mru.tempDoc.onload=k}cdc.mru.tempDoc.src=j;jQuery(cdc.mru.tempDoc).hide();setTimeout("window.location.href='"+g+"'",cdc.mru.timeOutMsecs);return false}};cdc.mru.timeOutMsecs=50000;jQuery(document).ready(function(){if(cdc.util.authStatus=="valid"){cdc.userInfoDispatcher.getUserProfile({listOfDataFields:["contactInfo"],callback:function(c){var d="";if(c.contactInfo!=null&&!jQuery.isEmptyObject(c.contactInfo)){if(c.contactInfo.givenname!=null&&c.contactInfo.givenname){d=c.contactInfo.givenname}if(c.contactInfo.sn!=null&&c.contactInfo.sn){d=d+" "+c.contactInfo.sn}}if(jQuery.isEmptyObject(c.contactInfo)||(!c.contactInfo.givenname&&!c.contactInfo.sn)){d="Logged In"}if(d){if(d!="Logged In"){d="Welcome, "+d}if(jQuery("header#fw-masthead")[0]){jQuery("#fw-utility #actions li.fw-welcome").html(d)}else{jQuery(".ft-toolbar span.ft-cq-welcome").html(d)}}}})}});
/*!
 * BEGIN /cdc/fw/m/ext_rules-lib.js
 */
(function(j){cdc.util.addMetricsRule=function(k,m,l){return c(k,m,l)};cdc.util.getMetricsInfo=function(k){var l=g(k);var o=k.name;if(o&&o.indexOf("&lpos=")===0){j(o.split("&")).each(function(){var n=j(this.split("=")).map(function(){return unescape(this)});if(n[0]==="lpos"&&!l.lpos){l.lpos=n[1]}if(n[0]==="lid"&&!l.lid){l.lid=n[1]}})}var m={"data-config-metrics-group":"lpos","data-config-metrics-title":"lid","data-config-metrics-item":"linktext"};l=f(k,m,l);return l};var d=[],h=function(k){return typeof k==="function"};function g(l,m){var k={};m=m||d;j(m).each(function(){var o=this.cond,p=this.vals,n=this.subs;if(!(h(o)?o(l):j(l).is(o))){return}j.extend(k,h(p)?p(l):p);if(n.length<1){return}j.extend(k,g(l,n))});return k}function c(l,n,m,o){n=n||{};o=o||d;var k=[];var p={cond:l,vals:n,subs:k};o.push(p);if(h(m)){m({subrule:function(q,t,r){return c(q,t,r,k)}})}}function f(m,l,n){for(var o in l){var k=j(m).attr(o);if(typeof k!==typeof undefined&&k!==false){n[l[o]]=j(m).attr(o)}}return n}})(jQuery);
/*!
 * BEGIN /cdc/fw/m/ext_liveManager.js
 */
if(typeof cdcPageTimers!="undefined"){cdcPageTimers.utloader=new Date()}if(typeof(cdc)=="undefined"){cdc=new Object()}if(typeof(cdc.util)=="undefined"){cdc.util=new Object()}if(typeof(cdc.util.ensureNamespace)=="undefined"){cdc.util.ensureNamespace=function(d){if(!d){return}var f=d.split(".");var g=window;for(var c=0;c<f.length;c++){var h=f[c];if(typeof(g[h])!="object"){g[h]=new Object()}g=g[h]}}}cdc.util.ensureNamespace("cdc.ut.liveManager");cdc.util.ensureNamespace("cdc.ut.trackEvent");cdc.util.ensureNamespace("cdc.ut.eventqueue");cdc.util.ensureNamespace("cdc.ut.config");cdc.ut.config.set=function(f,d,g){g={};if(typeof f=="string"){g[f]=d}else{g=f}for(f in g){if(typeof g[f]!="undefined"){cdc.ut.config[f]=g[f]}}};cdc.ut.liveManager={q:{},filesToLoad:[],fileIsLoaded:{},loadedFileCount:0,o:0,lmIsLoaded:0,img:[],loadFiles:function(fileInformation){if(fileInformation.notAsync){this.filesToLoad.push(fileInformation)}if(document.createElement){var nodeId="Lm_"+fileInformation.nodeIdSuffix;if(!document.getElementById(nodeId)){try{eval(fileInformation.initScript)}catch(exceptionObj){}var scriptNode=document.createElement("script");scriptNode.language="javascript";scriptNode.type="text/javascript";scriptNode.src=fileInformation.fileURL;scriptNode.id=nodeId;document.getElementsByTagName("head")[0].appendChild(scriptNode)}}},LOAD:function(libraryName){this.fileIsLoaded[libraryName]=0;var count=this.filesToLoad.length;for(var i=this.loadedFileCount;i<count;i++){var aFile=this.filesToLoad[i];if(this.fileIsLoaded[aFile.nodeIdSuffix]==0){this.fileIsLoaded[aFile.nodeIdSuffix]=++this.loadedFileCount;try{eval(aFile.callback)}catch(e){}}else{return}}if(this.loadedFileCount==count&&this.o==0){this.o=1}},bindEvent:function(f,c,g,d){if(f.addEventListener){f.addEventListener(c,g,false)}else{if(f.attachEvent){f.attachEvent(((d==1)?"":"on")+c,g)}}}};cdc.ut.liveManager.config={domain:".cisco.com",exit_domain:".cisco.com",passthru:true,sc_acct:"cisco-us",sc_test:false,sc_testaccount:"cisco-dev",dop_src:"//www.cisco.com/web/fw/m/ext_dop.min.js",dop_sensor:"//news-tags.cisco.com/tag/flashtag.gif?Log=1",dop_sensor_secure:"//news-tags.cisco.com/tag/auth/flashtag.gif?Log=1",s_code_src:"//www.cisco.com/web/fw/m/s_code.min.js",s_code_ut_src:"//www.cisco.com/web/fw/m/ext_s_code_ut.min.js",trackevent_src:"//www.cisco.com/web/fw/m/ext_trackEvent.min.js",visualsciences_ut_src:"//www.cisco.com/web/fw/m/ext_visualsciences_ut.min.js",rs_map_src:"//www.cisco.com/web/fw/m/ext_rs_map.min.js",ntpagetag_src:"//www.cisco.com/web/fw/m/ext_ntpagetag.min.js",ntpagetag_sensor:"//cisco-tags.cisco.com/tag/ntpagetag.gif",ntpagetag_sensor_secure:"//cisco-tags.cisco.com/tag/ntpagetag.gif",dev_domain:[],send:{dop:true,s_code:true,tnt:true,ntpagetag:false}};if(typeof cdc.homepage!="undefined"&&cdc.homepage.isHomepage==true){cdc.ut.liveManager.config.rs_map_src="//www.cisco.com/web/fw/m/rs_map.home.min.js";cdc.ut.liveManager.config.rs_map_src_cache="//www.cisco.com/web/fw/m/ext_rs_map.min.js"}cdc.ut.liveManager.lh=location.hostname;try{cdc.ut.liveManager.localConfig=cdc.util.ut.getConfigInfo();if(cdc.ut.liveManager.localConfig!==false){for(var i in cdc.ut.liveManager.localConfig){if(typeof cdc.ut.liveManager.localConfig[i]!="function"){if(i!="send"&&i!="dopconfig"&&i!="ntconfig"){cdc.ut.liveManager.config[i]=cdc.ut.liveManager.localConfig[i]}}}if(typeof cdc.homepage!="undefined"&&cdc.homepage.isHomepage==true){cdc.ut.liveManager.config.rs_map_src=cdc.ut.liveManager.localConfig.rs_map_home_src}if(typeof cdc.ut.liveManager.localConfig.dopconfig!="undefined"&&typeof cdc.ut.liveManager.localConfig.dopconfig.send!="undefined"&&typeof cdc.ut.liveManager.localConfig.dopconfig.send.dop_sensor!="undefined"){cdc.ut.liveManager.config.dop_sensor=cdc.ut.liveManager.localConfig.dopconfig.send.dop_sensor}try{cdc.ut.liveManager.config.dop_sensor=cdc.ut.liveManager.localConfig.dopconfig.send.dop_sensor}catch(e){}try{cdc.ut.liveManager.config.dop_sensor_secure=cdc.ut.liveManager.localConfig.dopconfig.send.dop_sensor_secure}catch(e){}try{cdc.ut.liveManager.config.ntpagetag_sensor=cdc.ut.liveManager.localConfig.ntconfig.send.ntpagetag_sensor}catch(e){}try{cdc.ut.liveManager.config.ntpagetag_sensor_secure=cdc.ut.liveManager.localConfig.ntconfig.send.ntpagetag_sensor_secure}catch(e){}if(typeof cdc.ut.liveManager.localConfig.send!="undefined"){for(var i in cdc.ut.liveManager.localConfig.send){if(typeof cdc.ut.liveManager.localConfig.send[i]!="function"){cdc.ut.liveManager.config.send[i]=cdc.ut.liveManager.localConfig.send[i]}}}}}catch(e){}cdc.ut.liveManager.storeRequest=function(g,c){try{if(cdc.ut.liveManager.config.store_requests[g]==true&&sessionStorage){var f=sessionStorage.getItem(g+"_rc");if(!f){f=0}sessionStorage.setItem(g+"_r"+f,c);sessionStorage.setItem(g+"_rc",parseInt(f)+1)}}catch(d){}};if(typeof cdc.ut.eventqueue.q=="undefined"){cdc.ut.eventqueue.q=[]}cdc.ut.trackEvent={event:function(g,f){var h="";for(var j in f){h+="&"+j+"="+f[j]}f.tag=cdc.ut.liveManager.trackingVariables.tag;cdc.ut.eventqueue.q.push({a:g,b:f})}};var trackEvent=cdc.ut.trackEvent;cdc.ut.liveManager.pageLocation=location.hostname+location.pathname;cdc.ut.liveManager.du=(document.URL.split("?"))[0];if(cdc.ut.liveManager.du.indexOf("#")>0&&cdc.ut.liveManager.du.lastIndexOf("/")>cdc.ut.liveManager.du.indexOf("#")){cdc.ut.liveManager.pageLocation+=location.hash}cdc.ut.liveManager.subDomains=new Array("www.","ecmx-wip.","ecmx-staging.","ecmx-active.","wwwin-tools-stage.","cco-rtp-1.","dev-stage.","tools-stage.","wwwin-tools-dev.");for(cdc.ut.liveManager.i=0;cdc.ut.liveManager.i<cdc.ut.liveManager.subDomains.length;cdc.ut.liveManager.i++){cdc.ut.liveManager.pageLocation=cdc.ut.liveManager.pageLocation.replace(cdc.ut.liveManager.subDomains[cdc.ut.liveManager.i],"")}if(cdc.ut.liveManager.pageLocation.lastIndexOf("/")==cdc.ut.liveManager.pageLocation.length-1){cdc.ut.liveManager.pageLocation=cdc.ut.liveManager.pageLocation.substring(0,cdc.ut.liveManager.pageLocation.length-1)}cdc.ut.liveManager.trackingVariables={basepage:cdc.ut.liveManager.pageLocation,property:"Cisco",tag:"ut2.0.20100823.1200",title:document.title,url:document.URL,referrer:eval("document.referrer"),linktrack:"linkpage",elementtype:"page"};try{if(typeof cdc.login=="undefined"){if(cdc.cookie.getCookie("wasOnLoginPage")=="true"){cdc.ut.liveManager.trackingVariables.referrer=cdc.cookie.getCookie("loginPageReferrer");cdc.cookie.setCookie({cookieName:"wasOnLoginPage",cookieValue:"false",domain:".cisco.com"})}}else{cdc.cookie.setCookie({cookieName:"wasOnLoginPage",cookieValue:"true",domain:".cisco.com"});cdc.cookie.setCookie({cookieName:"loginPageReferrer",cookieValue:document.referrer,domain:".cisco.com"})}}catch(e){}cdc.ut.liveManager.trackingVariables.loc="http://"+cdc.ut.liveManager.trackingVariables.basepage;cdc.ut.liveManager.getCookieData=function(f){var c=document.cookie.indexOf(f+"=");var d="";if(c>-1){d=document.cookie.indexOf(";",c+1);d=(d>0)?d:document.cookie.length;d=(d>c)?document.cookie.substring(c+f.length+1,d):""}return d};cdc.ut.liveManager.getPrevpage=function(){var x=eval("document.referrer");x=x.replace(/^https:\/\/|^http:\/\//,"");x=x.replace(/^www\./,"");var a=x.split("?");x=a[0];if(x.charAt(x.length-1)=="/"){x=x.substring(0,x.length-1)}if(x.length>255){x=x.substring(0,255)}return x};cdc.ut.liveManager.trackingVariables.prevpage=cdc.ut.liveManager.getPrevpage();cdc.ut.liveManager.presend=function(){try{var x=["campaign","country_site","position","referring_site","creative","keyword"];var y=location.search;if(y==""){var k=document.URL;var m=k.indexOf("#");var l=k.indexOf("?");if(l>m){y="?"+(k.split("?"))[1]}}y=y.toLowerCase();cdc.ut.liveManager.searchString=y;var v,t,r,q,p=[];for(v=0;v<x.length;v++){if(y.indexOf(x[v])>0){v=y.substring(1).split("&");t={};for(q=0;q<v.length;q++){r=v[q].split("=");t[r[0]]=unescape(r[1])}r="&tag="+cdc.ut.liveManager.trackingVariables.tag+"&vs_basepage="+escape(cdc.ut.liveManager.trackingVariables.basepage)+"&vs_elementtype=page&vs_event=campaign&vs_url="+escape(document.URL);for(q=0;q<x.length;q++){if(typeof t[x[q]]!="undefined"){r+="&"+x[q]+"="+escape(t[x[q]]);p.push(escape(t[x[q]]))}}q=document.cookie;var j=q.indexOf("CP_GUTC"),h=q.indexOf(";",j),u="";if(h>j&&j>=0){u=q.substring(j+8,h)}r+="&vs_vid="+u;var n=cdc.ut.liveManager.config.dop_sensor.split(",");var w=0;for(var o=0;o<n.length;o++){w=cdc.ut.liveManager.img.push(new Image());cdc.ut.liveManager.img[w-1].src=n[o]+r}break}}}catch(p){}};if(typeof lm_libloadedflag=="undefined"){cdc.ut.liveManager.presend()}cdc.ut.liveManager.ONLOAD=function(){if(typeof cdc.ut.config!="undefined"){for(a in cdc.ut.config){if(typeof cdc.ut.config[a]!="function"){b=a.split(".");if(b.length==2&&typeof cdc.ut.liveManager.config[b[0]][b[1]]!="undefined"){cdc.ut.liveManager.config[b[0]][b[1]]=cdc.ut.config[a]}else{if(b.length==1&&typeof cdc.ut.liveManager.config[b[0]]!="undefined"){cdc.ut.liveManager.config[b[0]]=cdc.ut.config[a]}}}}if(typeof cdc.ut.config.tag!="undefined"){cdc.ut.liveManager.t.tag=cdc.ut.config.tag}if(typeof cdc.ut.config.noload!="undefined"&&(cdc.ut.config.noload=="true"||cdc.ut.config.noload==true)){cdc.ut.liveManager.lmIsLoaded=1}}if(cdc.ut.liveManager.lmIsLoaded==1){return}cdc.ut.liveManager.lmIsLoaded=1;if(cdc.ut.liveManager.config.rs_map_src!=""){cdc.ut.liveManager.loadFiles({nodeIdSuffix:"rs_map",fileURL:cdc.ut.liveManager.config.rs_map_src+"?v="+cdc.ut.liveManager.trackingVariables.tag,notAsync:0})}if(typeof cdc.homepage!="undefined"&&cdc.homepage.isHomepage==true){try{if(cdc.ut.liveManager.config.rs_map_src_cache){cdc.ut.liveManager.loadFiles({nodeIdSuffix:"rs_map_cache",fileURL:cdc.ut.liveManager.config.rs_map_src_cache+"?v="+cdc.ut.liveManager.trackingVariables.tag,notAsync:0})}}catch(g){}}var d={flag:0,loc:document.URL,url:["mike.tealium.com","/en/US/docs/","/cisco/software/"],test:function(){for(var j=0;j<this.url.length;j++){if(this.loc.indexOf(this.url[j])>-1){this.flag=1;break}}}};d.test();if(!d.flag&&cdc.ut.liveManager.config.visualsciences_ut_src!=""){cdc.ut.liveManager.loadFiles({nodeIdSuffix:"vs",fileURL:cdc.ut.liveManager.config.visualsciences_ut_src+"?v="+cdc.ut.liveManager.trackingVariables.tag,notAsync:1})}if(cdc.ut.liveManager.config.dop_src!=""){cdc.ut.liveManager.loadFiles({nodeIdSuffix:"dop",fileURL:cdc.ut.liveManager.config.dop_src+"?v="+cdc.ut.liveManager.trackingVariables.tag,notAsync:1,callback:"cdc.ut.liveManager.dop.INIT()"})}if((typeof s_account=="undefined")||(s.visitorNamespace=="webex")){var h=cdc.ut.liveManager.config.s_code_ut_src;var f=(""+location.pathname).toLowerCase();var c=(new RegExp(/\/web\/\w{2}\//)).exec(f);if(c!=null){c=c.toString();if(c.indexOf("/web/us/")==-1&&c.indexOf("/web/fw/")==-1){h=cdc.ut.liveManager.config.s_code_src}}if(h!=""){cdc.ut.liveManager.loadFiles({nodeIdSuffix:"s_code",fileURL:h+"?v="+cdc.ut.liveManager.trackingVariables.tag,notAsync:1})}}else{cdc.ut.liveManager.config.send.s_code=false}if(cdc.ut.liveManager.config.ntpagetag_src!=""){if(typeof NTPT_IMGSRC=="undefined"){cdc.ut.liveManager.loadFiles({nodeIdSuffix:"ntpagetag",fileURL:cdc.ut.liveManager.config.ntpagetag_src+"?v="+cdc.ut.liveManager.trackingVariables.tag,notAsync:1,callback:"NTPT_UT.init()",initScript:"NTPT_NOINITIALTAG='true'"})}}if(cdc.ut.liveManager.config.trackevent_src!=""){cdc.ut.liveManager.loadFiles({nodeIdSuffix:"trackEvent",fileURL:cdc.ut.liveManager.config.trackevent_src+"?v="+cdc.ut.liveManager.trackingVariables.tag,notAsync:1,callback:"cdc.ut.liveManager.getPageData();cdc.ut.trackEvent.INIT();if(typeof cdcPageTimers!='undefined')cdcPageTimers.utsent=new Date();cdc.ut.trackEvent.event('view',cdc.ut.liveManager.trackingVariables)"})}if(typeof cdcPageTimers!="undefined"){cdcPageTimers.utloaded=new Date()}};cdc.ut.liveManager.ONERROR=function(f,d,g){if(cdc.ut.liveManager.erf!=1){cdc.ut.liveManager.error=(typeof f=="string")?(f+"-"+g):"Unknown";cdc.ut.liveManager.erf=1}};cdc.ut.liveManager.bindEvent(window,"error",cdc.ut.liveManager.ONERROR);cdc.ut.liveManager.getPageData=function(){var o={description:1,keywords:1,date:1,pubdate:1,pushdate:1,docRequest:1,synonym:1};var t=document.getElementsByTagName("meta");for(var y=0;y<t.length;y++){if(t[y].name&&t[y].name!=""&&typeof o[t[y].name]=="undefined"){cdc.ut.liveManager.trackingVariables["meta."+t[y].name.toLowerCase()]=t[y].content.toLowerCase()}}var g=document.getElementsByTagName("div");var j=[];for(var w=0;w<g.length;w++){if(g[w].className=="hinav"){var h=g[w].innerHTML.split("<a");for(var u=0;u<h.length;u++){if(h[u].indexOf('class="parent"')>-1||h[u].indexOf('class="selected"')>-1){j.push(h[u])}}var p=[];for(var v=0;v<j.length;v++){j[v]=(j[v].split("&amp;")).join("&");j[v]=j[v].toLowerCase();var l=j[v].indexOf(">");var k=j[v].indexOf("<",u);p.push(j[v].substring(l+1,k))}cdc.ut.liveManager.trackingVariables.hinav=p.join("/");break}}if(typeof document.getElementById("nav-treecrumb")!="undefined"){var d=document.getElementsByTagName("li");var r=[];for(w=0;w<d.length;w++){var n=d[w].className;var A=d[w].parentNode.id;if(A=="nav-treecrumb"){A=d[w].innerHTML;if(A){var m=A.indexOf(">");var x=A.indexOf("<",u);A=A.substring(m+1,x);r.push((A.split("&amp;")).join("&"))}}if(n=="crumb-selected"){break}}cdc.ut.liveManager.trackingVariables.treecrumb=r.join("/")}if(typeof track!="undefined"){for(var z in track){if(typeof z!="function"){cdc.ut.liveManager.trackingVariables[z]=track[z]}}}var q=["meta.iapath","hinav","treecrumb"];for(u=0;u<q.length;q++){if(typeof cdc.ut.liveManager.trackingVariables[q[u]]!="undefined"){cdc.ut.liveManager.trackingVariables.sitearea=cdc.ut.liveManager.trackingVariables[q[u]];cdc.ut.liveManager.trackingVariables.sa_source=q[u];break}}if(typeof lpMTagConfig!="undefined"&&typeof lpMTagConfig.sessionVar!="undefined"){lpMTagConfig.sessionVarBackup=new Array();lpMTagConfig.sessionVarBackup=lpMTagConfig.sessionVar}};cdc.ut.liveManager.getRSMapFileData=function(){if(typeof ut_rs_map!="undefined"){var c=document.URL;for(i in ut_rs_map){if(typeof ut_rs_map[i]=="string"){if(c.indexOf(i)>-1){return ut_rs_map[i]}}}}return""};(function(){var j="/c/dam/cdc/t/cdtm.js";var c="";var h="//fdk-devint.cisco.com";var f="//fdk-stage.cisco.com";var g="//www.cisco.com";function k(m){var l=document.createElement("script");l.src=m;l.async=false;document.getElementsByTagName("head")[0].appendChild(l)}function d(){if(typeof cdc!=="undefined"&&typeof cdc.util.findEnvironment!=="undefined"){var l=cdc.util.findEnvironment();switch(l){case"dev":c=h;break;case"stage":c=f;break;case"prod":c=g;break;case"unknown":c=g;break}}else{c=g}var m=c+j;k(m)}d()})();var lm_libloadedflag=1;cdc.ut.liveManager.debugThis=function(c,d){d=new Image();d.src="//cdn.tealium.com/track.gif?tag=ut2.0&msg="+c};cdc.ut.liveManager.t=cdc.ut.liveManager.trackingVariables;cdc.ut.liveManager.A=cdc.ut.liveManager.loadFiles;cdc.ut.liveManager.l=cdc.ut.liveManager.filesToLoad;for(var i=0;i<cdc.ut.liveManager.filesToLoad.length;i++){cdc.ut.liveManager.filesToLoad[i].a=cdc.ut.liveManager.filesToLoad[i].nodeIdSuffix;cdc.ut.liveManager.filesToLoad[i].b=cdc.ut.liveManager.filesToLoad[i].fileURL;cdc.ut.liveManager.filesToLoad[i].c=cdc.ut.liveManager.filesToLoad[i].notAsync;cdc.ut.liveManager.filesToLoad[i].d=cdc.ut.liveManager.filesToLoad[i].initScript;cdc.ut.liveManager.filesToLoad[i].e=cdc.ut.liveManager.filesToLoad[i].callback}cdc.ut.liveManager.cleanCookies=function(){var f=document.cookie;var d=f.split("; ");for(var l=0;l<d.length;l++){var c=d[l].split("=");if((c[0].indexOf("dgad_")==0)&&(c[0].indexOf(" ")>-1)){document.cookie=c[0]+"=;path=/;domain=.cisco.com;expires=Thu, 31 Dec 2009 00:00:00 GMT";c[0]=(c[0].split(" ")).join("_");document.cookie=c[0]+"="+c[1]+";path=/;domain=.cisco.com;"}else{if(c[0].indexOf("dgad_")==0){document.cookie=c[0]+"=;path=/;domain=.cisco.com;expires=Thu, 31 Dec 2009 00:00:00 GMT";document.cookie=c[0]+"="+c[1]+";path=/;domain=.cisco.com;"}}}if(f.length>3500){var k={};cdc.util.ensureNamespace("cdc.ut.cdcLargeCookies");f=f.split("; ");for(var l=0;l<f.length;l++){var g=f[l].split("=");k[g[0]]=g[1];if(g[1].length>250){cdc.ut.cdcLargeCookies[g[0]]=g[1].length}}cdc.ut.cdcLargeCookies.cdc_ut=0;cdc.ut.cdcLargeCookies.mbox=0;cdc.ut.cdcLargeCookies.dgad=0;cdc.ut.cdcLargeCookies.sc=0;cdc.ut.cdcLargeCookies.utm=0;for(var l in k){if(typeof k[l]!="function"){if((l=="cdc_ut")||(l=="mbox")||(l.indexOf("dgad_")==0)||(l.indexOf("s_")==0)||(l.indexOf("__utm")==0)){var h=l;if(h.indexOf("dgad_")==0){h="dgad"}if(h.indexOf("s_")==0){h="sc"}if(h.indexOf("__utm")==0){h="utm"}if((typeof cdc.ut.cdcLargeCookies[l]!="undefined")&&(cdc.ut.cdcLargeCookies[l]!=0)){cdc.ut.cdcLargeCookies[l]+=":d"}else{cdc.ut.cdcLargeCookies[h]+=k[l].length}document.cookie=l+"=;path=/;domain=.cisco.com;expires=Thu, 31 Dec 2009 00:00:00 GMT"}}}}};try{cdc.ut.liveManager.cleanCookies()}catch(e){}if(typeof cdc!="undefined"&&typeof cdc.includer!="undefined"&&typeof cdc.includer.alreadyInPage=="object"){cdc.includer.alreadyInPage.push("/cdc/fw/j/ext_util_core.js","/cdc/fw/m/ext_rules-lib.js","/cdc/fw/m/ext_liveManager.js")};