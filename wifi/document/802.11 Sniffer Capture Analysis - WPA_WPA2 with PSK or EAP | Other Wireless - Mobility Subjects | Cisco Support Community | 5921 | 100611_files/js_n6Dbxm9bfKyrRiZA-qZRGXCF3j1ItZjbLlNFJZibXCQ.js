(function ($) {

  Drupal.behaviors.esi = {
    attach: function (context, settings) {
      var contextualize_urls = (typeof Drupal.settings.ESI !== 'undefined') && Drupal.settings.ESI.contextualize_URLs;
      if (contextualize_urls) {
        $().esiTags().DrupalESIContextualizeURL().handleESI();
      }
      else {
        $().esiTags().handleESI();
      }
    }
  };

  // Add the ESI context from the cookie to the ESI URL.
  $.fn.DrupalESIContextualizeURL = function() {
    // Regex to find the ESI context from an ESI src URL.
    var esi_context_regexp = /\/CACHE%3D([^\/]+)$/;

    $(this).each(function() {
      src = $(this).attr('src');
      
      // Get the context key from the ESI src attribute.
      // Contexts are provided in the last element: e.g. /CACHE=FOO.
      if (match = $(this).attr('src').match(esi_context_regexp)) {
        context_key = match[1];
        context_val = esi_get_user_context(context_key);
        new_src = src.replace(esi_context_regexp, '/CACHE=' + context_val);
        $(this).attr('src', new_src);
      }
    });
    return this;
  }

})(jQuery);

/**
 * Get the value of a particular context.
 */
function esi_get_user_context(context_key) {
  // Transform a context string (e.g. 'USER') into the actual cookie name.
  cookie_name = Drupal.settings.ESI.cookie_prefix + context_key + Drupal.settings.ESI.cookie_suffix;
  return esi_get_value_from_cookie(cookie_name);
}

/**
 * Get the value of a named cookie.
 */
function esi_get_value_from_cookie(cookie_name) {
  var all_cookies = document.cookie.split(';');
  for (var i=0; i < all_cookies.length; i++) {
    if (all_cookies[i].indexOf(cookie_name) === 1) {
      var values = all_cookies[i].split('=');
      return values[1];
    }
  }
  return '';
}
;
/**
 * jQuery ESI Plugin v1.0
 * http://github.com/manarth/jquery_esi
 *
 * Dual licensed under the MIT and GPL licenses:
 *   http://www.opensource.org/licenses/mit-license.php
 *   http://www.gnu.org/licenses/gpl.html
 *
 * Usage:
 * $().esiTags().handleESI();
 */

(function ($) {

  // Fetch all ESI tags.
  $.fn.esiTags = function() {
    // Initialise a jQuery collection to return.
    esi_tags = $();

    // The handler can be called as $().esiTags, or $('foo').esiTags().
    // Ensure we have an iterable collection in either instance.
    if (this.length == 0) {
      collection = $('html');
    }
    else {
      collection = this;
    }

    collection.each(function() {
      // Retrieve the base DOM element, in order to access the DOM method
      // getElementsByTagName.
      base_element = $(this).get(0);
      // Discover the <esi:include> tags.
      jQuery.each(base_element.getElementsByTagName('esi:include'), function(i, val) {
        // Some DOMs fail to recognise that the ESI include tag is self-
        // closing, so they treat following tags as child nodes.
        if ($(this).children().length > 0) {
          // Move the child nodes to become siblings.
          children = $(this).children().detach();
          $(this).after(children);
        }
        esi_tags = esi_tags.add($(this));
      });
      // Discover the <esi:remove> tags.
      jQuery.each(base_element.getElementsByTagName('esi:remove'), function(i, val) {
        // ESI remove tags are not self-closing, so no special treatment for
        // child nodes is needed.
        esi_tags = esi_tags.add($(this));
      });
    });
    return esi_tags;
  };

  // Move child nodes to siblings.
  $.fn.handleESIChildren = function() {
    $(this).each(function() {
    });
    return this;
  }

  // Handle ESI tags.
  // Delegates to either .handleESIInclude() or .handleESIRemove() as needed.
  $.fn.handleESI = function() {
    $(this).each(function() {
      switch (this.nodeName.toLowerCase()) {
        case 'esi:include':
          $(this).handleESIInclude();
          break;
      
        case 'esi:remove':
          $(this).handleESIRemove();
          break;
      }
    });

    return this;
  }

  // Handle a single ESI Include tag.
  $.fn.handleESIInclude = function() {
    src = $(this).attr('src');
    jQuery.ajax({
      context: this,
      url: src,
      success: function(data, textStatus, jqXHR) {
        esiElement = $(this);
        esiElement.replaceWith(data);
      }
    });
  }

  // Handle a single ESI Remove tag.
  $.fn.handleESIRemove = function() {
    $(this).replaceWith('');
  }

})(jQuery);
;
(function ($) {

Drupal.prettify = Drupal.prettify || {};
  
/**
 * Attach prettify loader behavior.
 */
Drupal.behaviors.prettify = {
  attach: function (context) {  
    if (Drupal.settings.prettify.match) {
      context = Drupal.settings.prettify.match;
    }
  
    if (Drupal.settings.prettify.markup['code']) {
      // Selector for <code>...</code>
      $("code:not(.prettyprint)", context).not($("pre > code", context)).each(function () {
        Drupal.prettify.prettifyBlock($(this));
      });
    }
    if (Drupal.settings.prettify.markup['pre']) {
      // Selector for <pre>...</pre>
      $("pre:not(.prettyprint)", context).each(function () {
        Drupal.prettify.prettifyBlock($(this));
      });
    }
    else if (Drupal.settings.prettify.markup['precode']) {
      // Selector for <pre><code>...</code></pre>
      $("pre:not(.prettyprint) > code", context).parent().each(function () {
        Drupal.prettify.prettifyBlock($(this));
      });
    }
    
    // Process custom markup selectors
    for (var i = 0; i < Drupal.settings.prettify.custom.length; i++) {
      var selector = Drupal.settings.prettify.custom[i];
      if (selector) {
        $(selector, context).each(function () {
          if (!$(this).hasClass('prettyprint')) {
            codeBlock = $(this).parent().is('pre') ? $(this).parent() : $(this);
            Drupal.prettify.prettifyBlock(codeBlock);
          }
        });
      }      
    }
  
    if ($(".prettyprint").length > 0) {
      prettyPrint();
    }
  }
};

Drupal.prettify.prettifyBlock = function(codeBlock) {
  if (!codeBlock.hasClass(Drupal.settings.prettify.nocode)) {
    codeBlock.addClass("prettyprint");
    if (Drupal.settings.prettify.linenums && codeBlock.is('pre')) {
      codeBlock.addClass("linenums");
    }
  }
}

})(jQuery);
;
/**
 * Timeago is a jQuery plugin that makes it easy to support automatically
 * updating fuzzy timestamps (e.g. "4 minutes ago" or "about 1 day ago").
 *
 * @name timeago
 * @version 1.4.1
 * @requires jQuery v1.2.3+
 * @author Ryan McGeary
 * @license MIT License - http://www.opensource.org/licenses/mit-license.php
 *
 * For usage and examples, visit:
 * http://timeago.yarp.com/
 *
 * Copyright (c) 2008-2015, Ryan McGeary (ryan -[at]- mcgeary [*dot*] org)
 */

(function (factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define(['jquery'], factory);
  } else {
    // Browser globals
    factory(jQuery);
  }
}(function ($) {
  $.timeago = function(timestamp) {
    if (timestamp instanceof Date) {
      return inWords(timestamp);
    } else if (typeof timestamp === "string") {
      return inWords($.timeago.parse(timestamp));
    } else if (typeof timestamp === "number") {
      return inWords(new Date(timestamp));
    } else {
      return inWords($.timeago.datetime(timestamp));
    }
  };
  var $t = $.timeago;

  $.extend($.timeago, {
    settings: {
      refreshMillis: 60000,
      allowPast: true,
      allowFuture: false,
      localeTitle: false,
      cutoff: 0,
      strings: {
        prefixAgo: null,
        prefixFromNow: null,
        suffixAgo: "ago",
        suffixFromNow: "from now",
        inPast: 'any moment now',
        seconds: "less than a minute",
        minute: "about a minute",
        minutes: "%d minutes",
        hour: "about an hour",
        hours: "about %d hours",
        day: "a day",
        days: "%d days",
        month: "about a month",
        months: "%d months",
        year: "about a year",
        years: "%d years",
        wordSeparator: " ",
        numbers: []
      }
    },

    inWords: function(distanceMillis) {
      if(!this.settings.allowPast && ! this.settings.allowFuture) {
          throw 'timeago allowPast and allowFuture settings can not both be set to false.';
      }

      var $l = this.settings.strings;
      var prefix = $l.prefixAgo;
      var suffix = $l.suffixAgo;
      if (this.settings.allowFuture) {
        if (distanceMillis < 0) {
          prefix = $l.prefixFromNow;
          suffix = $l.suffixFromNow;
        }
      }

      if(!this.settings.allowPast && distanceMillis >= 0) {
        return this.settings.strings.inPast;
      }

      var seconds = Math.abs(distanceMillis) / 1000;
      var minutes = seconds / 60;
      var hours = minutes / 60;
      var days = hours / 24;
      var years = days / 365;

      function substitute(stringOrFunction, number) {
        var string = $.isFunction(stringOrFunction) ? stringOrFunction(number, distanceMillis) : stringOrFunction;
        var value = ($l.numbers && $l.numbers[number]) || number;
        return string.replace(/%d/i, value);
      }

      var words = seconds < 45 && substitute($l.seconds, Math.round(seconds)) ||
        seconds < 90 && substitute($l.minute, 1) ||
        minutes < 45 && substitute($l.minutes, Math.round(minutes)) ||
        minutes < 90 && substitute($l.hour, 1) ||
        hours < 24 && substitute($l.hours, Math.round(hours)) ||
        hours < 42 && substitute($l.day, 1) ||
        days < 30 && substitute($l.days, Math.round(days)) ||
        days < 45 && substitute($l.month, 1) ||
        days < 365 && substitute($l.months, Math.round(days / 30)) ||
        years < 1.5 && substitute($l.year, 1) ||
        substitute($l.years, Math.round(years));

      var separator = $l.wordSeparator || "";
      if ($l.wordSeparator === undefined) { separator = " "; }
      return $.trim([prefix, words, suffix].join(separator));
    },

    parse: function(iso8601) {
      var s = $.trim(iso8601);
      s = s.replace(/\.\d+/,""); // remove milliseconds
      s = s.replace(/-/,"/").replace(/-/,"/");
      s = s.replace(/T/," ").replace(/Z/," UTC");
      s = s.replace(/([\+\-]\d\d)\:?(\d\d)/," $1$2"); // -04:00 -> -0400
      s = s.replace(/([\+\-]\d\d)$/," $100"); // +09 -> +0900
      return new Date(s);
    },
    datetime: function(elem) {
      var iso8601 = $t.isTime(elem) ? $(elem).attr("datetime") : $(elem).attr("title");
      return $t.parse(iso8601);
    },
    isTime: function(elem) {
      // jQuery's `is()` doesn't play well with HTML5 in IE
      return $(elem).get(0).tagName.toLowerCase() === "time"; // $(elem).is("time");
    }
  });

  // functions that can be called via $(el).timeago('action')
  // init is default when no action is given
  // functions are called with context of a single element
  var functions = {
    init: function(){
      var refresh_el = $.proxy(refresh, this);
      refresh_el();
      var $s = $t.settings;
      if ($s.refreshMillis > 0) {
        this._timeagoInterval = setInterval(refresh_el, $s.refreshMillis);
      }
    },
    update: function(time){
      var parsedTime = $t.parse(time);
      $(this).data('timeago', { datetime: parsedTime });
      if($t.settings.localeTitle) $(this).attr("title", parsedTime.toLocaleString());
      refresh.apply(this);
    },
    updateFromDOM: function(){
      $(this).data('timeago', { datetime: $t.parse( $t.isTime(this) ? $(this).attr("datetime") : $(this).attr("title") ) });
      refresh.apply(this);
    },
    dispose: function () {
      if (this._timeagoInterval) {
        window.clearInterval(this._timeagoInterval);
        this._timeagoInterval = null;
      }
    }
  };

  $.fn.timeago = function(action, options) {
    var fn = action ? functions[action] : functions.init;
    if(!fn){
      throw new Error("Unknown function name '"+ action +"' for timeago");
    }
    // each over objects here and call the requested function
    this.each(function(){
      fn.call(this, options);
    });
    return this;
  };

  function refresh() {
    //check if it's still visible
    if(!$.contains(document.documentElement,this)){
      //stop if it has been removed
      $(this).timeago("dispose");
      return this;
    }

    var data = prepareData(this);
    var $s = $t.settings;

    if (!isNaN(data.datetime)) {
      if ( $s.cutoff == 0 || Math.abs(distance(data.datetime)) < $s.cutoff) {
        $(this).text(inWords(data.datetime));
      }
    }
    return this;
  }

  function prepareData(element) {
    element = $(element);
    if (!element.data("timeago")) {
      element.data("timeago", { datetime: $t.datetime(element) });
      var text = $.trim(element.text());
      if ($t.settings.localeTitle) {
        element.attr("title", element.data('timeago').datetime.toLocaleString());
      } else if (text.length > 0 && !($t.isTime(element) && element.attr("title"))) {
        element.attr("title", text);
      }
    }
    return element.data("timeago");
  }

  function inWords(date) {
    return $t.inWords(distance(date));
  }

  function distance(date) {
    return (new Date().getTime() - date.getTime());
  }

  // fix for IE6 suckage
  document.createElement("abbr");
  document.createElement("time");
}));
;
(function ($) {
Drupal.behaviors.suggestedTerms = {
  attach: function (context) {
    // get all attributes of span tag
    $('span.suggestedterm').each ( function() {
      var a = $('<a>' + this.innerHTML + '</a>')
      .attr('href', '#')
      .addClass($(this).attr('class'))
      .bind('click', function(event) {
        event.preventDefault();
        var input = $(this).parents(".form-item").find('input[type=text]');
        var text = $(this).text();
        // add the string
        if (((', ' + input.val() + ',').indexOf(', ' + text + ',') < 0) && ((', ' + input.val() + ',').indexOf(', "' + text + '",') < 0)) {
          if ((input.val()).length > 0) {
            input.val(input.val() + ', ');
          }
          input.val(input.val() + text);
          $(this).addClass('remove');
        }
        // remove the string
        else {
          var field_text = input.val();
          var string_to_remove = $(this).text();

          // Remove term if it's the only term selected so far
          if (string_to_remove == field_text) {
            input.val('');
            $(this).removeClass('remove');
          }
          else {
            // Remove term if it's at the beggining or in the middle of a series comma separated terms
            if (field_text.indexOf(string_to_remove + ', ') > -1) {
              var replacement_text = field_text.replace(string_to_remove + ', ', '');
              input.val(replacement_text);
              $(this).removeClass('remove');
            }

            // Remove the string if it's at the end of the series of terms.
            else if (position = field_text.indexOf(', ' + string_to_remove)) {
              var length_of_field_text = field_text.length;
              var length_of_string_to_remove = string_to_remove.length;

              // This test ensures the last term in the series is not just a
              // substring of the term we want to remove.
              if ((position + 2 + length_of_string_to_remove) == length_of_field_text) {
                var replacement_text = field_text.replace(', ' + string_to_remove, '');
                input.val(replacement_text);
                $(this).removeClass('remove');
              }
            }
          }
        } // end of add vs. remove
        }); // end bind
      $(this).before(a).remove();
    }); // end span.suggestedterm
  } // end of attach:
} // end of Drupal.behaviors.suggestedTerms
})(jQuery);
;
/**
 * @file
 * Some basic behaviors and utility functions for Views.
 */
(function ($) {

Drupal.Views = {};

/**
 * jQuery UI tabs, Views integration component
 */
Drupal.behaviors.viewsTabs = {
  attach: function (context) {
    if ($.viewsUi && $.viewsUi.tabs) {
      $('#views-tabset').once('views-processed').viewsTabs({
        selectedClass: 'active'
      });
    }

    $('a.views-remove-link').once('views-processed').click(function(event) {
      var id = $(this).attr('id').replace('views-remove-link-', '');
      $('#views-row-' + id).hide();
      $('#views-removed-' + id).attr('checked', true);
      event.preventDefault();
   });
  /**
    * Here is to handle display deletion
    * (checking in the hidden checkbox and hiding out the row)
    */
  $('a.display-remove-link')
    .addClass('display-processed')
    .click(function() {
      var id = $(this).attr('id').replace('display-remove-link-', '');
      $('#display-row-' + id).hide();
      $('#display-removed-' + id).attr('checked', true);
      return false;
  });
  }
};

/**
 * Helper function to parse a querystring.
 */
Drupal.Views.parseQueryString = function (query) {
  var args = {};
  var pos = query.indexOf('?');
  if (pos != -1) {
    query = query.substring(pos + 1);
  }
  var pairs = query.split('&');
  for(var i in pairs) {
    if (typeof(pairs[i]) == 'string') {
      var pair = pairs[i].split('=');
      // Ignore the 'q' path argument, if present.
      if (pair[0] != 'q' && pair[1]) {
        args[decodeURIComponent(pair[0].replace(/\+/g, ' '))] = decodeURIComponent(pair[1].replace(/\+/g, ' '));
      }
    }
  }
  return args;
};

/**
 * Helper function to return a view's arguments based on a path.
 */
Drupal.Views.parseViewArgs = function (href, viewPath) {
  var returnObj = {};
  var path = Drupal.Views.getPath(href);
  // Ensure we have a correct path.
  if (viewPath && path.substring(0, viewPath.length + 1) == viewPath + '/') {
    var args = decodeURIComponent(path.substring(viewPath.length + 1, path.length));
    returnObj.view_args = args;
    returnObj.view_path = path;
  }
  return returnObj;
};

/**
 * Strip off the protocol plus domain from an href.
 */
Drupal.Views.pathPortion = function (href) {
  // Remove e.g. http://example.com if present.
  var protocol = window.location.protocol;
  if (href.substring(0, protocol.length) == protocol) {
    // 2 is the length of the '//' that normally follows the protocol
    href = href.substring(href.indexOf('/', protocol.length + 2));
  }
  return href;
};

/**
 * Return the Drupal path portion of an href.
 */
Drupal.Views.getPath = function (href) {
  href = Drupal.Views.pathPortion(href);
  href = href.substring(Drupal.settings.basePath.length, href.length);
  // 3 is the length of the '?q=' added to the url without clean urls.
  if (href.substring(0, 3) == '?q=') {
    href = href.substring(3, href.length);
  }
  var chars = ['#', '?', '&'];
  for (i in chars) {
    if (href.indexOf(chars[i]) > -1) {
      href = href.substr(0, href.indexOf(chars[i]));
    }
  }
  return href;
};

})(jQuery);
;
/**
 * @file
 * Handles AJAX fetching of views, including filter submission and response.
 */
(function ($) {

/**
 * Attaches the AJAX behavior to Views exposed filter forms and key View links.
 */
Drupal.behaviors.ViewsAjaxView = {};
Drupal.behaviors.ViewsAjaxView.attach = function() {
  if (Drupal.settings && Drupal.settings.views && Drupal.settings.views.ajaxViews) {
    $.each(Drupal.settings.views.ajaxViews, function(i, settings) {
      Drupal.views.instances[i] = new Drupal.views.ajaxView(settings);
    });
  }
};

Drupal.views = {};
Drupal.views.instances = {};

/**
 * Javascript object for a certain view.
 */
Drupal.views.ajaxView = function(settings) {
  var selector = '.view-dom-id-' + settings.view_dom_id;
  this.$view = $(selector);

  // Retrieve the path to use for views' ajax.
  var ajax_path = Drupal.settings.views.ajax_path;

  // If there are multiple views this might've ended up showing up multiple times.
  if (ajax_path.constructor.toString().indexOf("Array") != -1) {
    ajax_path = ajax_path[0];
  }

  // Check if there are any GET parameters to send to views.
  var queryString = window.location.search || '';
  if (queryString !== '') {
    // Remove the question mark and Drupal path component if any.
    var queryString = queryString.slice(1).replace(/q=[^&]+&?|&?render=[^&]+/, '');
    if (queryString !== '') {
      // If there is a '?' in ajax_path, clean url are on and & should be used to add parameters.
      queryString = ((/\?/.test(ajax_path)) ? '&' : '?') + queryString;
    }
  }

  this.element_settings = {
    url: ajax_path + queryString,
    submit: settings,
    setClick: true,
    event: 'click',
    selector: selector,
    progress: { type: 'throbber' }
  };

  this.settings = settings;

  // Add the ajax to exposed forms.
  this.$exposed_form = this.$view.children('.view-filters').children('form');
  this.$exposed_form.once(jQuery.proxy(this.attachExposedFormAjax, this));

  // Add the ajax to pagers.
  this.$view
    // Don't attach to nested views. Doing so would attach multiple behaviors
    // to a given element.
    .filter(jQuery.proxy(this.filterNestedViews, this))
    .once(jQuery.proxy(this.attachPagerAjax, this));

  // Add a trigger to update this view specifically. In order to trigger a
  // refresh use the following code.
  //
  // @code
  // jQuery('.view-name').trigger('RefreshView');
  // @endcode
  // Add a trigger to update this view specifically.
  var self_settings = this.element_settings;
  self_settings.event = 'RefreshView';
  this.refreshViewAjax = new Drupal.ajax(this.selector, this.$view, self_settings);
};

Drupal.views.ajaxView.prototype.attachExposedFormAjax = function() {
  var button = $('input[type=submit], button[type=submit], input[type=image]', this.$exposed_form);
  button = button[0];

  this.exposedFormAjax = new Drupal.ajax($(button).attr('id'), button, this.element_settings);
};

Drupal.views.ajaxView.prototype.filterNestedViews= function() {
  // If there is at least one parent with a view class, this view
  // is nested (e.g., an attachment). Bail.
  return !this.$view.parents('.view').size();
};

/**
 * Attach the ajax behavior to each link.
 */
Drupal.views.ajaxView.prototype.attachPagerAjax = function() {
  this.$view.find('ul.pager > li > a, th.views-field a, .attachment .views-summary a')
  .each(jQuery.proxy(this.attachPagerLinkAjax, this));
};

/**
 * Attach the ajax behavior to a singe link.
 */
Drupal.views.ajaxView.prototype.attachPagerLinkAjax = function(id, link) {
  var $link = $(link);
  var viewData = {};
  var href = $link.attr('href');
  // Construct an object using the settings defaults and then overriding
  // with data specific to the link.
  $.extend(
    viewData,
    this.settings,
    Drupal.Views.parseQueryString(href),
    // Extract argument data from the URL.
    Drupal.Views.parseViewArgs(href, this.settings.view_base_path)
  );

  // For anchor tags, these will go to the target of the anchor rather
  // than the usual location.
  $.extend(viewData, Drupal.Views.parseViewArgs(href, this.settings.view_base_path));

  this.element_settings.submit = viewData;
  this.pagerAjax = new Drupal.ajax(false, $link, this.element_settings);
};

Drupal.ajax.prototype.commands.viewsScrollTop = function (ajax, response, status) {
  // Scroll to the top of the view. This will allow users
  // to browse newly loaded content after e.g. clicking a pager
  // link.
  var offset = $(response.selector).offset();
  // We can't guarantee that the scrollable object should be
  // the body, as the view could be embedded in something
  // more complex such as a modal popup. Recurse up the DOM
  // and scroll the first element that has a non-zero top.
  var scrollTarget = response.selector;
  while ($(scrollTarget).scrollTop() == 0 && $(scrollTarget).parent()) {
    scrollTarget = $(scrollTarget).parent();
  }
  // Only scroll upward
  if (offset.top - 10 < $(scrollTarget).scrollTop()) {
    $(scrollTarget).animate({scrollTop: (offset.top - 10)}, 500);
  }
};

})(jQuery);
;
(function ($) {

/**
 * Display the modal dialog.
 */
Drupal.behaviors.functions = {
	loadModal: function(title, href) {
		$("#pageModal").modal({
			show: true,
			backdrop: false,
		});

		// Reset the widgets.
		$('#pageModal').on('hidden', function() {
				$('.modal-header h3').empty();
				//$('.modal-body').html(default_body);
		});

		// Load the title and the modal.
		$('.modal-header h3').html(title);
		
		$(".modal-body").load(href);
		
		
		$("a#edit-cancel").click(function(e) {
			e.preventDefault();
			$("#pageModal").modal("toggle");
		});
		
		//Drupal.behaviors.functions.closeModal();

	},

}

})(jQuery);
;
(function ($) {
  Drupal.behaviors.rate = {
    attach: function(context) {
      $('.rate-widget:not(.rate-processed)', context).addClass('rate-processed').each(function () {
        var widget = $(this);
        // as we use drupal_html_id() to generate unique ids
        // we have to truncate the '--<id>'
        var ids = widget.attr('id').split('--');
        ids = ids[0].match(/^rate\-([a-z]+)\-([0-9]+)\-([0-9]+)\-([0-9])$/);
        var data = {
          content_type: ids[1],
          content_id: ids[2],
          widget_id: ids[3],
          widget_mode: ids[4]
        };

        $('a.rate-button', widget).click(function() {
          var token = this.getAttribute('href').match(/rate\=([a-zA-Z0-9\-_]{32,64})/)[1];
          return Drupal.rateVote(widget, data, token);
        });
      });
    }
  };

  Drupal.rateVote = function(widget, data, token) {
    // Invoke JavaScript hook.
    widget.trigger('eventBeforeRate', [data]);

    $(".rate-info", widget).text(Drupal.t('Saving vote...'));

    // Random number to prevent caching, see http://drupal.org/node/1042216#comment-4046618
    var random = Math.floor(Math.random() * 99999);

    var q = (Drupal.settings.rate.basePath.match(/\?/) ? '&' : '?') + 'widget_id=' + data.widget_id + '&content_type=' + data.content_type + '&content_id=' + data.content_id + '&widget_mode=' + data.widget_mode + '&token=' + token + '&destination=' + encodeURIComponent(Drupal.settings.rate.destination) + '&r=' + random;
    if (data.value) {
      q = q + '&value=' + data.value;
    }

    // fetch all widgets with this id as class
    widget = $('.' + widget.attr('id'));

    $.get(Drupal.settings.rate.basePath + q, function(response) {
      if (response.match(/^https?\:\/\/[^\/]+\/(.*)$/)) {
        // We got a redirect.
        document.location = response;
      }
      else {
        // get parent object
        var p = widget.parent();

        // Invoke JavaScript hook.
        widget.trigger('eventAfterRate', [data]);

        widget.before(response);

        // remove widget
        widget.remove();
        widget = undefined;

        Drupal.attachBehaviors(p);
      }
    });

    return false;
  }
})(jQuery);
;
(function ($) {
  Drupal.behaviors.rate_fivestar = {
    attach: function(context) {
      $('.rate-widget-fivestar ul:not(.rate-fivestar-processed)', context).addClass('rate-fivestar-processed').each(function () {
        var $this = $(this);
        // Save the current vote status

        var status = $('li a.rate-fivestar-btn-filled', $this).length;

        $this.children().hover(
            function()
            {
                // Append rate-fivestar-btn-filled class to all the a-elements except the a-elements after the hovered element
                var $this = $(this);
                $this.siblings().children('a').addClass('rate-fivestar-btn-filled').removeClass('rate-fivestar-btn-empty');
                $this.children('a').addClass('rate-fivestar-btn-filled').removeClass('rate-fivestar-btn-empty');
                $this.nextAll().children('a').removeClass('rate-fivestar-btn-filled').addClass('rate-fivestar-btn-empty');
            },
            function()
            {
                // Restore the current vote status
                $(this).parent().children().children('a').removeClass('rate-fivestar-btn-filled').addClass('rate-fivestar-btn-empty');
                $(this).parent().children().slice(0,status).children('a').removeClass('rate-fivestar-btn-empty').addClass('rate-fivestar-btn-filled');
            }
        );
      });
    }
  };
})(jQuery);
;
