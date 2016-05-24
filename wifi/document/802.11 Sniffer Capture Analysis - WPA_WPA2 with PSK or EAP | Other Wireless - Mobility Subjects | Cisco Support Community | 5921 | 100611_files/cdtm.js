(function cdtmClosure () {
    // so we can leave the global namespace better than we found it.
var activeProfile = "";
var loc = window.location.href; // "http://www.cisco.com";
loc = loc.toLowerCase();
try {
	var currScrpt = document.currentScript; // refers to current loaded script
} catch(e) {
	console.log(e);
}

if (typeof cdc !== "undefined" && typeof cdc.homepage !== "undefined" && cdc.homepage.isHomepage) {
    activeProfile = "home";
}

function setEnv () {
    if (typeof cdc != "undefined" && typeof cdc.util.findEnvironment != "undefined") {
        var thisEnv = cdc.util.findEnvironment();
        switch (thisEnv) {
            case "dev":
            return "dev";
            break;
            case "stage":
            return "qa";
            break;
            case "prod":
            return "prod";
            break;
            case "unknown":
            return "prod";
            break;
        }
    }
}
function getMetricsSrcInfo(){ // get metrics info source, which loaded cdtm.js
	var metricsSrcInfo = "";
	try {		
		if(currScrpt.getAttribute('data-config-src') !=null){
			metricsSrcInfo = currScrpt.getAttribute('data-config-src');		 
		}else{
			metricsSrcInfo = checkSnapshot();
		}
		return metricsSrcInfo;
	} catch(e) {
		console.log(e);
		return metricsSrcInfo; // returns empty string in case of any JS exception
	}
	
}
function checkSnapshot() { // utilty used by getMetricsSrcInfo for checking snapshot versions
	try {
		var scrptTagResp = document.querySelector('script[src*="/etc/designs/cdc/fw/snapshots/js/responsive-"]');
		var scrptTagSate = document.querySelector('script[src*="/etc/designs/cdc/fw/snapshots/js/satellite-"]');
		//console.log(scrptTagResp);
		var snapShot, version;
		snapShot = "";
		if (scrptTagResp != null) {
			//console.log("get the version of responive");
			version = parseFloat(scrptTagResp.getAttribute("src").match(/-(.*)v\.js/)[1]);
			snapShot = (version > 1.10) ? "Snapshot-Responsive-" + version : "";
			//console.log(version);
		} else if (scrptTagSate != null) {
			version = parseFloat(scrptTagSate.getAttribute("src").match(/-(.*)v\.js/)[1]);
			snapShot = (version > 1.10) ? "Snapshot-Satellite-" + version : "";
		}
		return snapShot;
	} catch(e) {
		console.log(e);
	}
	
}
var env = setEnv();
var srcInfo = getMetricsSrcInfo();

var config = {
    // url properties must be ordered from longest to shortest.,
    ".cisco.com/c/m/zh_cn/never-better/":"nbabt-cn",
    ".cisco.com/c/zh_cn/products/security/index.html":"nbabt-cn",
    ".cisco.com/c/zh_cn/solutions/enterprise-networks/cisco-digital-network-architecture.html":"nbabt-cn",
    ".cisco.com/c/zh_cn/products/hyperconverged-infrastructure/index.html":"nbabt-cn",
    ".cisco.com/c/m/":"microsite",
    "apps.cisco.com/Commerce/":"bnp",
    "apps.cisco.com/ccw":"bnp",
    "buildprice.cisco.com":"bnp",
    "mycase.cloudapps.cisco.com/":"support",
    "mycase-stage.cloudapps.cisco.com/":"support",
    "quickview-stage.cloudapps.cisco.com/":"support",
    "quickview.cloudapps.cisco.com/":"support",
    "bst-stage.cloudapps.cisco.com/bugsearch/":"support",
    "bst.cloudapps.cisco.com/bugsearch/":"support",
    "snmp-stage.cloudapps.cisco.com/support/snmp/":"support",
    "snmp.cloudapps.cisco.com/support/snmp/":"support",
    "cway.cisco.com":"support",
    "cway-stg.cisco.com":"support",
    "supportforums.cisco.com":"support",
    "csc-stage.cisco.com":"support",
    "dcpplat%.cloudapps.cisco.com":"microsite",
    "dcp%.cisco.com":"microsite",
    "onesearch.cisco.com":"cec",
    "onesearch-stage.cloudapps.cisco.com":"cec",
    "onesearch-dev.cloudapps.cisco.com":"cec",
    "apps.cisco.com":"cdtm",
    "sso.cisco.com":"cdtm",
    "techzone.cisco.com":"cdtm",
    "videolounge.cisco.com":"cdtm",
    ".acquisitionconnection.com":"cdtm",
    "webapps.cisco.com":"cdtm",
    "blogs.cisco.com":"blogs",
    "communities.cisco.com":"communities",
    ".cisco.com/c/r/":"microsite",
    "internetofeverything.cisco.com":"microsite",
    "ioeassessment.cisco.com":"microsite",
    "share.cisco.com":"microsite",
    "newsroom.cisco.com":"newsroom",
    "smbmarketplace.cisco.com":"commerce",
    "software.cisco.com":"support",
    "video.cisco.com":"video",
    "weare.cisco.com":"microsite",
    "socialmedia.cisco.com":"microsite",
    "marketplace.cisco.com":"microsite",
    "developer.cisco.com/site/devnet/":"microsite",
    "unleashedit.com":"microsite",
    "uberflip.com":"microsite",
    ".epuboffers.cisco.com":"microsite",
    "csr.cisco.com":"microsite",
    "offers.cisco.com":"microsite",
    "b2me.cisco.com":"microsite",
    "digital.cisco.com":"microsite",
    ".ciscolive.com":"microsite",
    "demand.cisco.com":"microsite",
    "lifeconnections.cisco.com":"microsite",
    "td.cloudsec.sco.cisco.com":"microsite",
    "td-stage.cloudsec.sco.cisco.com":"microsite",
    "b2me-cisco-com.p03.elqsandbox.com":"microsite",
    "info-ciscotest1-com.p03.elqsandbox.com":"microsite",
    "cognitive.cisco.com":"microsite",
    ".cisco.com/support/":"support",
    ".cisco.com/cpc/":"support",    
    ".cisco.com/servicerequesttool/":"support",
    ".cisco.com/bugsearch":"support",
    ".cisco.com/swift/":"support",
    ".cisco.com/itdit/cfn/":"support",
    ".cisco.com/gct/upgrade/":"support",
    ".cisco.com/itdit/mibs/servlet/":"support",
    "wwwin.cisco.com":"cec",
    ".cisco.com/c/cec/":"cec",
    "grs.cisco.com":"grs",
    ".cisco.com/cisco/web/uk/":"global",
    ".cisco.com/web/ca/":"global",
    ".cisco.com/web/br/":"global",
    ".cisco.com/web/ea/":"global",
    ".cisco.com/web/za/":"global",
    ".cisco.com/web/br/":"global",
    ".cisco.com/web/anz/":"global",
    ".cisco.com/web/jp/":"global",
    ".cisco.com/web/sg/":"global",
    ".cisco.com/web/cn/":"global",
    ".cisco.com/web/kr/":"global",
    ".cisco.com/web/tw/":"global",
    ".cisco.com/web/hk/":"global",
    ".cisco.com/web/my/":"global",
    ".cisco.com/web/th/":"global",
    ".cisco.com/web/in/":"global",
    ".cisco.com/web/vn/":"global",
    ".cisco.com/web/id/":"global",
    ".cisco.com/web/ph/":"global",
    ".cisco.com/web/at/":"global",
    ".cisco.com/web/gr/":"global",
    ".cisco.com/web/pt/":"global",
    ".cisco.com/web/az/":"global",
    ".cisco.com/web/hu/":"global",
    ".cisco.com/web/ro/":"global",
    ".cisco.com/web/be/":"global",
    ".cisco.com/web/ru/":"global",
    ".cisco.com/web/ba/":"global",
    ".cisco.com/web/it/":"global",
    ".cisco.com/web/yu/":"global",
    ".cisco.com/web/bg/":"global",
    ".cisco.com/web/kz/":"global",
    ".cisco.com/web/sk/":"global",
    ".cisco.com/web/hr/":"global",
    ".cisco.com/web/lv/":"global",
    ".cisco.com/web/si/":"global",
    ".cisco.com/web/cy/":"global",
    ".cisco.com/web/lt/":"global",
    ".cisco.com/web/es/":"global",
    ".cisco.com/web/cz/":"global",
    ".cisco.com/web/lu/":"global",
    ".cisco.com/web/se/":"global",
    ".cisco.com/web/dk/":"global",
    ".cisco.com/web/mk/":"global",
    ".cisco.com/web/ch/":"global",
    ".cisco.com/web/ee/":"global",
    ".cisco.com/web/mt/":"global",
    ".cisco.com/web/tr/":"global",
    ".cisco.com/web/fi/":"global",
    ".cisco.com/web/nl/":"global",
    ".cisco.com/web/ua/":"global",
    ".cisco.com/web/fr/":"global",
    ".cisco.com/web/no/":"global",
    ".cisco.com/web/de/":"global",
    ".cisco.com/web/pl/":"global",
    ".cisco.com/web/ar/":"global",
    ".cisco.com/web/ec/":"global",
    ".cisco.com/web/pa/":"global",
    ".cisco.com/web/bz/":"global",
    ".cisco.com/web/sv/":"global",
    ".cisco.com/web/py/":"global",
    ".cisco.com/web/bo/":"global",
    ".cisco.com/web/gt/":"global",
    ".cisco.com/web/pe/":"global",
    ".cisco.com/web/br/":"global",
    ".cisco.com/web/hn/":"global",
    ".cisco.com/web/pr/":"global",
    ".cisco.com/web/cl/":"global",
    ".cisco.com/web/la/":"global",
    ".cisco.com/web/uy/":"global",
    ".cisco.com/web/co/":"global",
    ".cisco.com/web/mx/":"global",
    ".cisco.com/web/ve/":"global",
    ".cisco.com/web/cr/":"global",
    ".cisco.com/web/ni/":"global",
    ".cisco.com/web/il/":"global",
    ".cisco.com/web/me/":"global",
    
    ".cisco.com/%/az_az/":"presales",
    ".cisco.com/%/bg_bg/":"presales",
    ".cisco.com/%/cs_cz/":"presales",
    ".cisco.com/%/de_at/":"presales",
    ".cisco.com/%/de_ch/":"presales",
    ".cisco.com/%/el_gr/":"presales",
    ".cisco.com/%/en_be/":"presales",
    ".cisco.com/%/en_ca/":"presales",
    ".cisco.com/%/en_cy/":"presales",
    ".cisco.com/%/en_dz/":"presales",
    ".cisco.com/%/en_hk/":"presales",
    ".cisco.com/%/en_id/":"presales",
    ".cisco.com/%/en_mt/":"presales",
    ".cisco.com/%/en_my/":"presales",
    ".cisco.com/%/en_ng/":"presales",
    ".cisco.com/%/en_ph/":"presales",
    ".cisco.com/%/en_sg/":"presales",
    ".cisco.com/%/en_vn/":"presales",
    ".cisco.com/%/en_za/":"presales",
    ".cisco.com/%/es_ar/":"presales",
    ".cisco.com/%/es_bo/":"presales",
    ".cisco.com/%/es_bz/":"presales",
    ".cisco.com/%/es_cl/":"presales",
    ".cisco.com/%/es_co/":"presales",
    ".cisco.com/%/es_cr/":"presales",
    ".cisco.com/%/es_ec/":"presales",
    ".cisco.com/%/es_gt/":"presales",
    ".cisco.com/%/es_hn/":"presales",
    ".cisco.com/%/es_mx/":"presales",
    ".cisco.com/%/es_ni/":"presales",
    ".cisco.com/%/es_pa/":"presales",
    ".cisco.com/%/es_pe/":"presales",
    ".cisco.com/%/es_pr/":"presales",
    ".cisco.com/%/es_py/":"presales",
    ".cisco.com/%/es_sv/":"presales",
    ".cisco.com/%/es_uy/":"presales",
    ".cisco.com/%/es_ve/":"presales",
    ".cisco.com/%/et_ee/":"presales",
    ".cisco.com/%/fi_fi/":"presales",
    ".cisco.com/%/fr_be/":"presales",
    ".cisco.com/%/fr_ca/":"presales",
    ".cisco.com/%/fr_ch/":"presales",
    ".cisco.com/%/fr_dz/":"presales",
    ".cisco.com/%/hr_hr/":"presales",
    ".cisco.com/%/hu_hu/":"presales",
    ".cisco.com/%/lt_lt/":"presales",
    ".cisco.com/%/lv_lv/":"presales",
    ".cisco.com/%/mk_mk/":"presales",
    ".cisco.com/%/nl_be/":"presales",
    ".cisco.com/%/pt_pt/":"presales",
    ".cisco.com/%/ro_ro/":"presales",
    ".cisco.com/%/ru_ua/":"presales",
    ".cisco.com/%/sk_sk/":"presales",
    ".cisco.com/%/sl_si/":"presales",
    ".cisco.com/%/sr_rs/":"presales",
    ".cisco.com/%/uk_ua/":"presales",
    ".cisco.com/%/vi_vn/":"presales",
    ".cisco.com/%/zh_hk/":"presales",
    ".cisco.com/%/zh_tw/":"presales",
    ".cisco.com/%/en_au/":"presales",
    ".cisco.com/%/pt_br/":"presales",
    ".cisco.com/%/zh_cn/":"presales",
    ".cisco.com/%/fr_fr/":"presales",
    ".cisco.com/%/ko_kr/":"presales",
    ".cisco.com/%/en_uk/":"presales",

    ".cisco.com/en/us/support/":"support",
    ".cisco.com/cisco/web/support/":"support",
    ".cisco.com/cisco/web/psa/":"support",
    ".cisco.com/cisco/web/br/support/":"support",
    ".cisco.com/cisco/web/br/psa/":"support",
    ".cisco.com/cisco/web/cn/support/":"support",
    ".cisco.com/cisco/web/cn/psa/":"support",
    ".cisco.com/cisco/web/jp/support/":"support",
    ".cisco.com/cisco/web/jp/psa/":"support",
    ".cisco.com/cisco/web/la/support/":"support",
    ".cisco.com/cisco/web/la/psa/":"support",
    ".cisco.com/cisco/web/ru/support/":"support",
    ".cisco.com/cisco/web/ru/psa/":"support",
    ".cisco.com/cisco/web/ca/fr/support/":"support",
    ".cisco.com/cisco/web/ca/fr/psa/":"support",

    ".cisco.com/c/%/support/":"support",
    ".cisco.com/c/%/td/":"support",
    "csc-%.cisco.com":"support",

    ".cisco.com/c/en/us/products":"presales",
    ".cisco.com/c/en/us/":"presales",
    ".cisco.com/web/applicat/dsprecal/":"support",
    ".cisco.com/web/tsweb/":"support",
    ".cisco.com/web/":"presales",
    "cisco.com/cisco/psn/services/asdc":"presales",
    ".cisco.com/cisco/psn":"cdtm",
    ".cisco.com/cisco/web/solutions/small_business/":"cdtm",
    ".cisco.com/cisco/web":"presales",
    ".youtubecisco.com":"social",

    ".cisco.com/cgi-bin/software/":"support",
    ".cisco.com/cgi-bin/xxipsnet/":"support",
    ".cisco.com/c/dam/":"presales",
    "tools-stage-was7.cisco.com/ys3bnx/":"commerce",
    "tools.cisco.com":"cdtm",
    ".connectedfuturesmag.com":"microsite",
    ".jasper.com":"microsite",

    ".":"cdtm"

};

var profileWhiteList = ["blogs", "cdtm", "chat", "communities", "global", "newsroom", "social", "video"];
// when a match is found between the config and whitelist, UT should still be activated.
var addUT = "";
if (activeProfile === "") {
    // If we already set a profile using a method other than a url match, we will skip this step.
    for (var f in config) {
        // check for profile before checking url
        if (config[f] !== "") {
            var pattern = f;
            pattern = pattern.replace(new RegExp("\\.","g"),"\\."); //convert all "."s in url to non-special using escape char
            pattern = pattern.replace(new RegExp("%","g"),".*?"); //Replace the wildcard "%" with ".*?" to match anything including blank("")
            var re = new RegExp(pattern);
            if (re.test(loc)) {
                activeProfile = config[f];
                break; // only one profile can be used per-match.
            }
        }
    }
}

function getUtag(activeProfile, env){
	if (typeof env == "undefined") {
		env = "prod";
	}	
	var a='//tags.tiqcdn.com/utag/cisco/' + activeProfile + '/' + env + '/utag.js';
	var b=document;
	var c='script';
	var d=b.createElement(c);
	var s= (srcInfo != "")? srcInfo:"cdtm";
	d.src=a;
	d.type='text/java'+c;
	d.setAttribute("data-config-src", s);
	d.async=true;
	a=b.getElementsByTagName(c)[0];
	a.parentNode.insertBefore(d,a);
};
/**
* live manager rs_map source config for nic.cisco
* this code can be removed from here, once 'nic.cisco' is listed in Prod domain list of 'util_core.js'
*/
if(location.hostname == 'nic.cisco'){
    try{
        cdc.ut.liveManager.config.rs_map_src = "//www.cisco.com/web/fw/m/rs_map.min.js";
    } catch(e) {};
}
/**
* Code to disable UT on specified location, using local storage concept used in livemanger
* This code can be removed once satellite.js framework is updated
*/

try {
        /*delete local storage if it already set */
       if (cdc.util.ut.getConfigInfo() !== false) {
        cdc.util.ls.deleteConfigInfo("utConfig");
        }
        if (location.pathname.indexOf("/c/r/en/us/internet-of-everything-ioe/") >=0){
        //if (location.pathname == "/c/r/en/us/internet-of-everything-ioe/tomorrow-starts-here/index.html") {
            var localConfig = {
                //load these any time the test loader is used. override these with values from unique configs
                //NOTE: these are NOT UT production defaults. They are specific to non-production environments

                domain: ".cisco.com", // domain value used to write cookies.
                exit_domain: "cisco.com", // used to qualify as exit link
                passthru: true, // DOP specific. pass any data received. [false (use map) | true (passthrough) ]
                // individual sub library load locations
                dop_src: "",
                s_code_src: "",
                s_code_ut_src: "",
                trackevent_src: "",
                visualsciences_ut_src: "",
                rs_map_src: "",
                rs_map_home_src: "",
                ntpagetag_src: ""

            };
            cdc.util.ls.setConfigInfo("utConfig", localConfig);
        }

} catch (e) {}
/*End of code to disable UT on Satellite pages*/

function loadUT() {
    try {
        jQuery(document).ready(function() {
                if(typeof cdcPageTimers!='undefined')cdcPageTimers.utready=new Date();
                    cdc.ut.liveManager.ONLOAD();
                })
    } catch(e) {
            cdc.ut.liveManager.bindEvent(window, "load", cdc.ut.liveManager.ONLOAD);
            setTimeout("cdc.ut.liveManager.ONLOAD", 3000);
    }
}
for (var f in profileWhiteList) {
    if (activeProfile === profileWhiteList[f]) {
        addUT = true;
        break;
    }
}
if (activeProfile !== "" && addUT === true) {
    getUtag(activeProfile,env);
    loadUT();
} else if (activeProfile === "pilot" && addUT !== true) {
    loadUT();
} else if (activeProfile === "cdtm" || activeProfile !== "") {
    getUtag(activeProfile,env);
}
// End of cdtmClosure;
})();
