(function ($) {
  Drupal.behaviors.cscMetrics = {
    /**
     * Set variables for metrics declaring that this user is anonymous.
     */
    anon: function () {
      NTPT_PGEXTRA = 'status=Anonymous';
    },

    /**
     * Set variables for metrics declaring that the user is authenticated.
     */
    loggedIn: function (username) {
      NTPT_PGEXTRA = 'un=' + username + '&status=LoggedIn';
    },

    /**
     * The attached behavior.
     */
    attach: function(context, settings) {
      if (typeof settings.user === "undefined" || settings.user == 0) {
        this.anon();
      }
      else {
        this.loggedIn(settings.username);
      }
    }
  };
})(jQuery);
;
