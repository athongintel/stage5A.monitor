//tealium universal tag - utag.83 ut4.0.201605192047, Copyright 2016 Tealium.com Inc. All Rights Reserved.
try{(function(id,loader){var u={};utag.o[loader].sender[id]=u;if(utag===undefined){utag={};}if(utag.ut===undefined){utag.ut={};}if(utag.ut.loader===undefined){u.loader=function(o){var a,b,c,l;a=document;if(o.type==="iframe"){b=a.createElement("iframe");b.setAttribute("height","1");b.setAttribute("width","1");b.setAttribute("style","display:none");b.setAttribute("src",o.src);}else if(o.type==="img"){utag.DB("Attach img: "+o.src);b=new Image();b.src=o.src;return;}else{b=a.createElement("script");b.language="javascript";b.type="text/javascript";b.async=1;b.charset="utf-8";b.src=o.src;}if(o.id){b.id=o.id;}if(typeof o.cb==="function"){if(b.addEventListener){b.addEventListener("load",function(){o.cb();},false);}else{b.onreadystatechange=function(){if(this.readyState==="complete"||this.readyState==="loaded"){this.onreadystatechange=null;o.cb();}};}}l=o.loc||"head";c=a.getElementsByTagName(l)[0];if(c){utag.DB("Attach to "+l+": "+o.src);if(l==="script"){c.parentNode.insertBefore(b,c);}else{c.appendChild(b);}}};}else{u.loader=utag.ut.loader;}
u.ev={'view':1};u.initialized=false;u.map={"cp.CP_GUTC":"gutc_id.pageAttribute","dom.title":"document.title.pageAttribute","page.page_taxonomy":"taxonomy.pageAttribute","marketingData.keycode_path":"keycode_path.pageAttribute","marketingData.campaignPath":"campaign_path.pageAttribute","visitorProfile.hourOfDay":"hour_of_day.pageAttribute","visitorProfile.dayOfWeek":"day_of_week.pageAttribute","visitorProfile.weekdayVSweekend":"weekday_vs_weekend.pageAttribute","visitorBehavior.prev_page":"prev_page.pageAttribute","page.iaPath_level_1":"ia_path_level_1.pageAttribute","page.iaPath_level_2":"ia_path_level_2.pageAttribute","page.iaPath_level_3":"ia_path_level_3.pageAttribute","page.iaPath_level_4":"ia_path_level_4.pageAttribute","page.language_country":"language_country.pageAttribute","page.theater":"theater.pageAttribute","page.iaPath_full":"ia_path_full.pageAttribute","visitorBehavior.link_type":"visitor_behavior_link_type.pageAttribute","page.pageName":"page_name.pageAttribute","visitorBehavior.link_hierarchy":"visitor_behavior_link_hierarchy.pageAttribute","js_page.utag.cfg.v":"utag.cfg.v.pageAttribute","page.derivedValues.report_suite_id":"report_suite_id.pageAttribute","visitorProfile.technology.screen_width":"screen_width.pageAttribute","qp.campaign":"campaign.pageAttribute","qp.creative":"creative.pageAttribute","qp.referring_site":"referrering_site.pageAttribute","marketingData.keycode_sc":"keycode_sc.pageAttribute","visitorBehavior.link_text":"visitor_behavior_link_text","visitorProfile.auth_state":"visitor_profile_auth_state.pageAttribute","visitorBehavior.link_id":"visitor_behavior_link_id.pageAttribute","visitorBehavior.link_pos":"visitor_behavior_link_pos.pageAttribute","qp.priority_code":"priority_code.pageAttribute","marketingData.agency_full":"agency_full.pageAttribute","page.content_type":"content_type.pageAttribute","visitorProfile.entitlement":"entitlement.pageAttribute","qp.country_site":"country_site.pageAttribute","qp.position":"position.pageAttribute","qp.keyword":"keyword.pageAttribute","page.channel":"channel.pageAttribute","visitorBehavior.pageView":"pageView.pageAttribute","technicalEnabler.download_track":"download_track.pageAttribute","visitorBehavior.click_to_chat":"click_to_chat.pageAttribute","marketingData.grs_viewed":"grs_viewed.pageAttribute","marketingData.grs_complete":"grs_complete.pageAttribute","marketingData.ad_clickthrough":"ad_clickthrough.pageAttribute","marketingData.landingpage_clickthrough":"landingpage_clickthrough.pageAttribute","marketingData.ad_clickthrough_keycode_present":"ad_clickthrough_keycode_present.pageAttribute","marketingData.landingpage_clickthrough_keycode_present":"landingpage_clickthrough_keycode_present.pageAttribute","meta.doctype":"doctype.pageAttribute","page.access_level":"access_level.pageAttribute","page.url_colon":"url_colon.pageAttribute","visitorBehavior.link_value_type":"visitor_behavior_link_value.pageAttribute","visitorBehavior.af_track":"visitor_behavior_af_track.pageAttribute","df_suite":"df_suite.pageAttribute","df_tag":"df_tag.pageAttribute","video_category":"video_category.pageAttribute","sales_stage":"sales_stage.pageAttribute","business_unit":"business_unit.pageAttribute","captioned":"captioned.pageAttribute","caption_language":"caption_language.pageAttribute","video_renderer":"video_renderer.pageAttribute","brightcove_video_id":"brightcove_video_id.pageAttribute","campaign_page":"campaign_page.pageAttribute","basepage":"basepage.pageAttribute","video_event":"video_event.pageAttribute","element_name":"element_name.pageAttribute","content_container":"content_container.pageAttribute","overlay_prop44":"overlay.pageAttribute","visitorBehavior.page_link":"visitor_behavior_page_link.pageAttribute","video_page":"video_page.pageAttribute","video_info":"video_info.pageAttribute","video_title":"video_title.pageAttribute","media_type":"media_type.pageAttribute","visitorBehavior.dlink_value":"visitor_behavior_dlink_value.pageAttribute","visitorBehavior.social_likes":"visitor_behavior_social_likes.pageAttribute","visitorBehavior.social_likes_name":"visitor_behavior_social_likes_name.pageAttribute","tab_param":"tab_param.pageAttribute","visitorBehavior.homePageView":"homePageView.pageAttribute","visitorBehavior.homePageLinkClick":"homePageLinkClick.pageAttribute","isOndemand":"isOndemand.pageAttribute","fge":"fge.pageAttribute","visitorBehavior.fgeLinkClick":"fgeLinkClick.pageAttribute","visitorBehavior.fgeImpression":"fgeImpression.pageAttribute","dom.url":"url.pageAttribute"};u.extend=[];u.send=function(a,b){if(u.ev[a]||u.ev.all!==undefined){var c={},d,e,f,i;u.data={"event":"","base_url":"//j.6sc.co/6si.min.js"
};for(d in utag.loader.GV(u.map)){if(b[d]!==undefined&&b[d]!==""){e=u.map[d].split(",");for(f=0;f<e.length;f++){if(e[f].indexOf('.pageAttribute')>-1){c[e[f].split('.pageAttribute')[0]]=b[d];}else{u.data[e[f]]=b[d];}}}}
u.data.token=u.data.token||"11e2668bb1b1e1cc32a5be6023423ac3";window._6si=window._6si||[];window._6si.push(['enableEventTracking',true,['A','BUTTON','VIDEO','OBJECT']]);window._6si.push(['setToken',u.data.token]);var formNames=[],formTags=document.getElementsByTagName('form'),fields=[];for(var i=0;i<formTags.length;i++){formNames.push(formTags[i].name);var formFields=formTags[0].getElementsByTagName('input');for(var j=0;j<formFields.length;j++){if(formFields[j].name){fields.push(formFields[j].name);}}}
window._6si.push(['setBlacklistFields',formNames.concat(fields)]);if(b['ut.env']&&b['ut.env']==='prod'){window._6si.push(['setEndpoint','b.6sc.co']);}else{window._6si.push(['setEndpoint','bqa.6sc.co']);}
if(a=='view'){function isEmpty(obj){for(var key in c){if(typeof c[key]!=='undefined'){return false;}}
return true;}
if(!isEmpty(c)){window._6si.push(['setPageAttributes',c]);}}
u.loader_cb=function(){u.initialized=true;};if(!u.initialized){u.loader({"type":"script","src":u.data.base_url,"cb":u.loader_cb,"loc":"script","id":'utag_83'});}else{u.loader_cb();}
}};utag.o[loader].loader.LOAD(id);})("83","cisco.support");}catch(error){utag.DB(error);}
