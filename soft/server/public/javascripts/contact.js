'use strict';

var contact = function() {

    this.getData = function() {
        var name, email, message, data;

        name    = $('#c_name').val();
        email   = $('#c_email').val();
        message = $('#c_message').val();

        data = {
            name: name,
            email: email,
            message: message
        };

        return data;
    }

    this.success = function(data) {
        $('#c_success').fadeIn();
    }

    this.send = function() {
        $.post(
            '/contact',
            this.getData(),
            this.success
        )
        $('#c_form').fadeOut();
    };
};

var c = new contact();

$('#c_form').submit(function(e) {
    e.preventDefault();

    c.send();
});
