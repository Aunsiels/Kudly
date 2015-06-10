package controllers

import play.api._
import play.api.mvc._
import play.api.data._
import play.api.data.Forms._

import play.api.libs.mailer._
import play.api.Play.current

object Contact extends Controller {

    /* The data when someone complete the contact us form */
    case class ContactData (name : String, email : String, message : String)

    /* Receive the form for the contact */
    val contactForm : Form[ContactData] = Form(
        mapping(
            "name"  -> nonEmptyText,
            "email" -> nonEmptyText,
            "message" -> nonEmptyText
        )(ContactData.apply)(ContactData.unapply))
   
    /* Receive the contact form and send an email */
    def contact = Action { implicit request =>
        contactForm.bindFromRequest.fold(
            error => {
                BadRequest("A problem happened")},
            c     => {
               val email = Email(
                   "Contact Site",
                   c.name + " <" + c.email +">",
                   Seq("Kudly <kudlyproject@gmail.com>"),
                   // adds attachment
                   attachments = Seq(),
                   // sends text, HTML or both...
                   bodyText = Some(c.name + " " + c.email + " " + c.message),
                   bodyHtml = None
                   )
               MailerPlugin.send(email)
               Redirect(routes.Application.index)
            }
        )
    }

}
