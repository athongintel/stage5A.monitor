(function ($) {
  Drupal.behaviors.featureContentModal = {
    attach: function(context, settings) {
      $(".action-unfeature, .action-feature").click(function(e) {
          e.preventDefault();
          var title = $(this).attr('title');
          var href = $(this).attr('href') + ' #cisco-core-feature-node';

          Drupal.behaviors.functions.loadModal(title, href);
      });

      $(".action-feature_expert_interviews, .action-unfeature_expert_interviews").click(function(e) {
          e.preventDefault();
          var title = $(this).attr('title');
          var href = $(this).attr('href') + ' #cisco-document-expert-interview-node';

          Drupal.behaviors.functions.loadModal(title, href);
      });
    }
  }
})(jQuery);
;
(function ($) {

	Drupal.behaviors.shareAffix = {
		attach: function(context, settings) {
			if (jQuery('.share').length > 0) {
				var top = jQuery('.share').offset().top - parseFloat(jQuery('.share').css('marginTop').replace(/auto/, 0));

				jQuery(window).scroll(function(event) {
					var y = jQuery(this).scrollTop();

					if(y >= top) {
						jQuery('.share').addClass('fixed');
					} else {
						jQuery('.share').removeClass('fixed');
					}
				});
			}
		}
	}

	// prevent submitting a comment form twice
	Drupal.behaviors.ciscoPreventDoublePost = {
		attach: function(context, settings) {
			$('#comment-form').submit(function(){
				$('button', this).attr('disabled','disabled');
			});
		}
	}

	Drupal.behaviors.toggleCollapse = {
		attach: function(context, settings) {
			$('.replies-toolbar').once('replies-toolbar', function() {
				$('.indented').each(function(){
					$(this).addClass('expanded').prev('.comment').addClass('parent-comment');
				});


			var collapse_trigger = function() {
				$(this).toggleClass('collapse-all expand-all');

				if($(this).text() == 'Collapse all') {

					$(this).text('Expand all');
					$(this).closest('#comments').find('.indented').addClass('collapsed');
					$(this).closest('#comments').find('.indented.expanded').slideToggle('fast');
					$(this).closest('#comments').find('.collapse-toggle').addClass('collapsed').text('See More');

				} else {

					$(this).text('Collapse all');
					$(this).closest('#comments').find('.indented').addClass('expanded');
					$(this).closest('#comments').find('.indented.collapsed').slideToggle('fast');
					$(this).closest('#comments').find('.collapse-toggle').addClass('expanded').text('Collapse');
				}

				return false;
			},
			collapse_toggle = function() {
				$(this).toggleClass('collapsed expanded').closest('.comment').next('.indented').slideToggle('fast').toggleClass('expanded collapsed');

				if($(this).text() == 'See More') {
					$(this).text('Collapse');
				} else {
					$(this).text('See More');
				}

				return false;
			};

			$('a.collapse-trigger').once('collapse-trigger', function() { $(this).click(collapse_trigger) });
			$('a.collapse-toggle').once('collapse-toggle', function() { $(this).click(collapse_toggle) });

	      if (typeof(Drupal.settings.cisco) != 'undefined' && Drupal.settings.cisco.comment.collapse == '1'){
	          $('a.collapse-trigger', this).click();
	      }
			});
		}
	}
})(jQuery);
;
(function($) {
  Drupal.behaviors.paneCollapse = {
    attach: function(context, settings) {
      $(".cisco-collapsible-pane").each(function() {
        var handle = $("h2.pane-title", this);
        var content = $(".pane-content-toggle", this);

        handle.once('slide-toggle', function() {
          handle.click(function() {
            var icon = $("i", this);

            content.slideToggle("fast");
            icon.toggleClass("icon-plus-sign icon-minus-sign", 200);
          });
        });
      });
    }
  }
})(jQuery);;
(function ($) {

  Drupal.behaviors.activityPane = {
    attach: function(context, settings) {
      // The height of the content block when it's not expanded
      var adjustheight = 20;
      // The "more" link text
      var moreText = "+";
      // The "less" link text
      var lessText = "-";

      // Sets the .more-less div to the specified height and hides any content that overflows
      $(".more-less").css('height', adjustheight).css('overflow', 'hidden');

      $("a.show-more-toggle").text(moreText);

      $('a.show-more-toggle').once('js-show-more-toggle', function() {
        $(this).toggle(function() {
          $(this).parents("div:first").find(".more-less").css('height', 'auto').css('overflow', 'visible');
          $(this).text(lessText);
          $(this).removeClass('show-more');
          $(this).addClass('show-less');
        }, function() {
          $(this).parents("div:first").find(".more-less").css('height', adjustheight).css('overflow', 'hidden');
          $(this).text(moreText);
          $(this).removeClass('show-less');
          $(this).addClass('show-more');
        });
      })
    }
  }

})(jQuery);
;
(function($){
/**
 * To make a form auto submit, all you have to do is 3 things:
 *
 * ctools_add_js('auto-submit');
 *
 * On gadgets you want to auto-submit when changed, add the ctools-auto-submit
 * class. With FAPI, add:
 * @code
 *  '#attributes' => array('class' => array('ctools-auto-submit')),
 * @endcode
 *
 * If you want to have auto-submit for every form element,
 * add the ctools-auto-submit-full-form to the form. With FAPI, add:
 * @code
 *   '#attributes' => array('class' => array('ctools-auto-submit-full-form')),
 * @endcode
 *
 * If you want to exclude a field from the ctool-auto-submit-full-form auto submission,
 * add the class ctools-auto-submit-exclude to the form element. With FAPI, add:
 * @code
 *   '#attributes' => array('class' => array('ctools-auto-submit-exclude')),
 * @endcode
 *
 * Finally, you have to identify which button you want clicked for autosubmit.
 * The behavior of this button will be honored if it's ajaxy or not:
 * @code
 *  '#attributes' => array('class' => array('ctools-use-ajax', 'ctools-auto-submit-click')),
 * @endcode
 *
 * Currently only 'select', 'radio', 'checkbox' and 'textfield' types are supported. We probably
 * could use additional support for HTML5 input types.
 */

Drupal.behaviors.CToolsAutoSubmit = {
  attach: function(context) {
    // 'this' references the form element
    function triggerSubmit (e) {
      var $this = $(this);
      if (!$this.hasClass('ctools-ajaxing')) {
        $this.find('.ctools-auto-submit-click').click();
      }
    }

    // the change event bubbles so we only need to bind it to the outer form
    $('form.ctools-auto-submit-full-form', context)
      .add('.ctools-auto-submit', context)
      .filter('form, select, input:not(:text, :submit)')
      .once('ctools-auto-submit')
      .change(function (e) {
        // don't trigger on text change for full-form
        if ($(e.target).is(':not(:text, :submit, .ctools-auto-submit-exclude)')) {
          triggerSubmit.call(e.target.form);
        }
      });

    // e.keyCode: key
    var discardKeyCode = [
      16, // shift
      17, // ctrl
      18, // alt
      20, // caps lock
      33, // page up
      34, // page down
      35, // end
      36, // home
      37, // left arrow
      38, // up arrow
      39, // right arrow
      40, // down arrow
       9, // tab
      13, // enter
      27  // esc
    ];
    // Don't wait for change event on textfields
    $('.ctools-auto-submit-full-form input:text, input:text.ctools-auto-submit', context)
      .filter(':not(.ctools-auto-submit-exclude)')
      .once('ctools-auto-submit', function () {
        // each textinput element has his own timeout
        var timeoutID = 0;
        $(this)
          .bind('keydown keyup', function (e) {
            if ($.inArray(e.keyCode, discardKeyCode) === -1) {
              timeoutID && clearTimeout(timeoutID);
            }
          })
          .keyup(function(e) {
            if ($.inArray(e.keyCode, discardKeyCode) === -1) {
              timeoutID = setTimeout($.proxy(triggerSubmit, this.form), 500);
            }
          })
          .bind('change', function (e) {
            if ($.inArray(e.keyCode, discardKeyCode) === -1) {
              timeoutID = setTimeout($.proxy(triggerSubmit, this.form), 500);
            }
          });
      });
  }
}
})(jQuery);
;
(function ($) {

	Drupal.behaviors.loadMessage = {
		attach: function(context, settings) {
			Drupal.behaviors.loadMessage.markMessageStatus(context);
		},

		markMessageStatus: function(context) {
			var threadContainer = $('#quicktabs-tabpage-profile_content-4');

			if ('undefined' !== typeof(threadContainer.on)) {
				threadContainer.on('click', '.update-message-status', function(e) {
					// Stop link redirect and multiple triggers from the same class.
					e.preventDefault();
					e.stopImmediatePropagation();

					$link = $(this);

					// Send POST request
					$.ajax({
						type: 'POST',
						url: e.target.href,
						data: { js: true },
						dataType: 'json',
						success: function (data) {
							// 0 == PRIVATEMSG_READ.
							// 1 == PRIVATEMSG_UNREAD.
							if (data.newStatus == 0) {
								$link.text(Drupal.t('Mark as read'));

								// Set up new url to mark as read.
								var url = $link.attr('href');
								var urlArray = url.split('/');
								urlArray[3] = "0";
								url = urlArray.join('/');
								$link.attr('href', url);
							}
							else {
								$link.text(Drupal.t('Mark as unread'));

								// Set up new url to mark as read.
								var url = $link.attr('href');
								var urlArray = url.split('/');
								urlArray[3] = "1";
								url = urlArray.join('/');
								$link.attr('href', url);
							}
						},
						error: function (xmlhttp) {
						}
					});
				});
			}
		}
	}

	$.fn.privateMessagesSetFilterSent = function(data) {
		$('#views-exposed-form-cisco-profile-private-message-block-1 #edit-is-new').val('sent');
	}

	/**
   * Add error highlighting for fields.
   *
   * @param {array} data
   *   Array of wrapper classes which have errors.
   */
  $.fn.ciscoMessagesHighlightErrors = function(errors) {
    $('.control-group').removeClass('error');

    if ((errors !== undefined) && errors.length > 0) {
      for (var i = 0; i < errors.length; i++) {
        $('div[class*=form-item-' + errors[i] + ']').addClass('error');
      }
    }
  }

})(jQuery);
;
(function ($) {

/**
 * Terminology:
 *
 *   "Link" means "Everything which is in flag.tpl.php" --and this may contain
 *   much more than the <A> element. On the other hand, when we speak
 *   specifically of the <A> element, we say "element" or "the <A> element".
 */

/**
 * The main behavior to perform AJAX toggling of links.
 */
Drupal.flagLink = function(context) {
  /**
   * Helper function. Updates a link's HTML with a new one.
   *
   * @param element
   *   The <A> element.
   * @return
   *   The new link.
   */
  function updateLink(element, newHtml) {
    var $newLink = $(newHtml);

    // Initially hide the message so we can fade it in.
    $('.flag-message', $newLink).css('display', 'none');

    // Reattach the behavior to the new <A> element. This element
    // is either whithin the wrapper or it is the outer element itself.
    var $nucleus = $newLink.is('a') ? $newLink : $('a.flag', $newLink);
    $nucleus.addClass('flag-processed').click(flagClick);

    // Find the wrapper of the old link.
    var $wrapper = $(element).parents('.flag-wrapper:first');
    if ($wrapper.length == 0) {
      // If no ancestor wrapper was found, or if the 'flag-wrapper' class is
      // attached to the <a> element itself, then take the element itself.
      $wrapper = $(element);
    }
    // Replace the old link with the new one.
    $wrapper.after($newLink).remove();
    Drupal.attachBehaviors($newLink.get(0));

    $('.flag-message', $newLink).fadeIn();
    setTimeout(function(){ $('.flag-message', $newLink).fadeOut() }, 3000);
    return $newLink.get(0);
  }

  /**
   * A click handler that is attached to all <A class="flag"> elements.
   */
  function flagClick() {
    // 'this' won't point to the element when it's inside the ajax closures,
    // so we reference it using a variable.
    var element = this;

    // While waiting for a server response, the wrapper will have a
    // 'flag-waiting' class. Themers are thus able to style the link
    // differently, e.g., by displaying a throbber.
    var $wrapper = $(element).parents('.flag-wrapper');
    if ($wrapper.is('.flag-waiting')) {
      // Guard against double-clicks.
      return false;
    }
    $wrapper.addClass('flag-waiting');

    // Hide any other active messages.
    $('span.flag-message:visible').fadeOut();

    // Emulate the AJAX data sent normally so that we get the same theme.
    var data = {};
    data['js'] = true;
    data['ajax_page_state[theme]'] = Drupal.settings.ajaxPageState.theme;
    data['ajax_page_state[theme_token]'] = Drupal.settings.ajaxPageState.theme_token;

    // Send POST request
    $.ajax({
      type: 'POST',
      url: element.href,
      data: data,
      dataType: 'json',
      success: function (data) {
        if (data.status) {
          // Success.
          data.link = $wrapper.get(0);
          $.event.trigger('flagGlobalBeforeLinkUpdate', [data]);
          if (!data.preventDefault) { // A handler may cancel updating the link.
            data.link = updateLink(element, data.newLink);
          }
          $.event.trigger('flagGlobalAfterLinkUpdate', [data]);
        }
        else {
          // Failure.
          alert(data.errorMessage);
          $wrapper.removeClass('flag-waiting');
        }
      },
      error: function (xmlhttp) {
        alert('An HTTP error '+ xmlhttp.status +' occurred.\n'+ element.href);
        $wrapper.removeClass('flag-waiting');
      }
    });
    return false;
  }

  $('a.flag-link-toggle:not(.flag-processed)', context).addClass('flag-processed').click(flagClick);
};

/**
 * Prevent anonymous flagging unless the user has JavaScript enabled.
 */
Drupal.flagAnonymousLinks = function(context) {
  $('a.flag:not(.flag-anonymous-processed)', context).each(function() {
    this.href += (this.href.match(/\?/) ? '&' : '?') + 'has_js=1';
    $(this).addClass('flag-anonymous-processed');
  });
}

String.prototype.flagNameToCSS = function() {
  return this.replace(/_/g, '-');
}

/**
 * A behavior specifically for anonymous users. Update links to the proper state.
 */
Drupal.flagAnonymousLinkTemplates = function(context) {
  // Swap in current links. Cookies are set by PHP's setcookie() upon flagging.

  var templates = Drupal.settings.flag.templates;

  // Build a list of user-flags.
  var userFlags = Drupal.flagCookie('flags');
  if (userFlags) {
    userFlags = userFlags.split('+');
    for (var n in userFlags) {
      var flagInfo = userFlags[n].match(/(\w+)_(\d+)/);
      var flagName = flagInfo[1];
      var contentId = flagInfo[2];
      // User flags always default to off and the JavaScript toggles them on.
      if (templates[flagName + '_' + contentId]) {
        $('.flag-' + flagName.flagNameToCSS() + '-' + contentId, context).after(templates[flagName + '_' + contentId]).remove();
      }
    }
  }

  // Build a list of global flags.
  var globalFlags = document.cookie.match(/flag_global_(\w+)_(\d+)=([01])/g);
  if (globalFlags) {
    for (var n in globalFlags) {
      var flagInfo = globalFlags[n].match(/flag_global_(\w+)_(\d+)=([01])/);
      var flagName = flagInfo[1];
      var contentId = flagInfo[2];
      var flagState = (flagInfo[3] == '1') ? 'flag' : 'unflag';
      // Global flags are tricky, they may or may not be flagged in the page
      // cache. The template always contains the opposite of the current state.
      // So when checking global flag cookies, we need to make sure that we
      // don't swap out the link when it's already in the correct state.
      if (templates[flagName + '_' + contentId]) {
        $('.flag-' + flagName.flagNameToCSS() + '-' + contentId, context).each(function() {
          if ($(this).find('.' + flagState + '-action').size()) {
            $(this).after(templates[flagName + '_' + contentId]).remove();
          }
        });
      }
    }
  }
}

/**
 * Utility function used to set Flag cookies.
 *
 * Note this is a direct copy of the jQuery cookie library.
 * Written by Klaus Hartl.
 */
Drupal.flagCookie = function(name, value, options) {
  if (typeof value != 'undefined') { // name and value given, set cookie
    options = options || {};
    if (value === null) {
      value = '';
      options = $.extend({}, options); // clone object since it's unexpected behavior if the expired property were changed
      options.expires = -1;
    }
    var expires = '';
    if (options.expires && (typeof options.expires == 'number' || options.expires.toUTCString)) {
      var date;
      if (typeof options.expires == 'number') {
        date = new Date();
        date.setTime(date.getTime() + (options.expires * 24 * 60 * 60 * 1000));
      } else {
        date = options.expires;
      }
      expires = '; expires=' + date.toUTCString(); // use expires attribute, max-age is not supported by IE
    }
    // NOTE Needed to parenthesize options.path and options.domain
    // in the following expressions, otherwise they evaluate to undefined
    // in the packed version for some reason...
    var path = options.path ? '; path=' + (options.path) : '';
    var domain = options.domain ? '; domain=' + (options.domain) : '';
    var secure = options.secure ? '; secure' : '';
    document.cookie = [name, '=', encodeURIComponent(value), expires, path, domain, secure].join('');
  } else { // only name given, get cookie
    var cookieValue = null;
    if (document.cookie && document.cookie != '') {
      var cookies = document.cookie.split(';');
      for (var i = 0; i < cookies.length; i++) {
        var cookie = jQuery.trim(cookies[i]);
        // Does this cookie string begin with the name we want?
        if (cookie.substring(0, name.length + 1) == (name + '=')) {
          cookieValue = decodeURIComponent(cookie.substring(name.length + 1));
          break;
        }
      }
    }
    return cookieValue;
  }
};

Drupal.behaviors.flagLink = {};
Drupal.behaviors.flagLink.attach = function(context) {
  // For anonymous users with the page cache enabled, swap out links with their
  // current state for the user.
  if (Drupal.settings.flag && Drupal.settings.flag.templates) {
    Drupal.flagAnonymousLinkTemplates(context);
  }

  // For all anonymous users, require JavaScript for flagging to prevent spiders
  // from flagging things inadvertently.
  if (Drupal.settings.flag && Drupal.settings.flag.anonymous) {
    Drupal.flagAnonymousLinks(context);
  }

  // On load, bind the click behavior for all links on the page.
  Drupal.flagLink(context);
};

})(jQuery);
;
