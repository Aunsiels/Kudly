package controllers

import play.api._
import play.api.mvc._
import play.api.data._
import play.api.data.Forms._
import java.io.File
import java.io.FileInputStream
import play.api.libs.iteratee._
import scala.concurrent.ExecutionContext.Implicits.global
import play.api.mvc._
import play.api.Play.current
import akka.actor._
import play.api.libs.concurrent.Execution.Implicits.defaultContext
import scala.concurrent._
import play.api.libs.json._

import com.mongodb.casbah.Imports._
import com.mongodb.casbah.gridfs.Imports._
import scala.language.postfixOps

object Application extends Controller {

    /* Database */
    val dataBase = MongoClient(
        MongoClientURI(
            "mongodb://kudly:19071993@ds041168.mongolab.com:41168/kudly"
        )
    ).getDB("kudly")

    /* Collection of raw data */
    val rawCollection = dataBase("Raw")

    /*
     * Contains a led description
     */
    case class Led (
        n     : Int,
        r     : Int,
        g     : Int,
        b     : Int
    )

    /*
     * Led form
     */
    val ledForm : Form[Led] = Form(
        mapping(
            "n" -> number(min = 0, max = 2),
            "r" -> number(min = 0, max = 255),
            "g" -> number(min = 0, max = 255),
            "b" -> number(min = 0, max = 255)
        )(Led.apply)(Led.unapply)
    )

    /*
     * Variables to ask for a photo or for streaming
     */
    var streamingRequest = 0;
    var photoRequest = 0;

    /*
     * Welcome page
     */
    def index = Action {
        Ok("Welcome page")
    }

    /*
     * Connexion fake through url
     */
    def connexion(pseudo : String, password : String) = Action {
         if (pseudo == "kudly" && password == "kudly"){
             Ok("The connexion succeeded")
         } else {
             Ok("Wrong pseudo/password")
         }
    }

    /*
     * Send a simple test for leds
     */
    def pwm = Action {
        val data = rawCollection.findOne("data" $eq "led")
        data match {
            case Some(led) => {
                var l = Led (
                    led.getAs[Int]("n").getOrElse(0),
                    led.getAs[Int]("r").getOrElse(0),
                    led.getAs[Int]("g").getOrElse(0),
                    led.getAs[Int]("b").getOrElse(0))
                Ok("<led>\n<pwm_set n=\""+
                   + l.n + "\" r=\""
                   + l.r + "\" g=\""
                   + l.g + "\" b=\""
                   + l.b + "\"/>\n</led>")}
            case None =>
                Ok("No value yet")
        }
    }

    /*
     * Stream an audio file throught a GET request
     */
    def stream = Action {
        val file = new File("public/0233.ogg")

        Ok.sendFile(file)
    }

    /* echo websocket */
    def echo = WebSocket.using[Array[Byte]] { request =>
        /* Channel to communicate */
        val (out, channel) = Concurrent.broadcast[Array[Byte]]
        val in = Iteratee.foreach[Array[Byte]] {
            msg => channel push msg
        }
        (in, out)
    }

    /* Echo Form */
    val echoForm : Form[String] = Form(
        "data" -> text
    )

    /*
     * Simply answer with what was received
     */
    def postEcho = Action { implicit request =>
        echoForm.bindFromRequest.fold(
            errors => {
                BadRequest("Bad data") },
            data   => {
                Ok("Receive data : " + data)
            }
        )
    }

    /*
     * Set the pwm value
     */
    def setPwm = Action { implicit request =>
        ledForm.bindFromRequest.fold(
            errors => {
                BadRequest("Wrong parameters")},
            led    => {
                rawCollection.remove("data" $eq "led")
                var ledData = MongoDBObject(
                    "data"  -> "led",
                    "n" -> led.n,
                    "r" -> led.r,
                    "g" -> led.g,
                    "b" -> led.b
                )
                rawCollection += ledData
                Ok("Led data received")
            }
        )
    }

    /*
     * Graph value representation
     */
    case class graphValue (date : java.util.Date,
                           value : Int) {
        /* Return a format Date */
        def getDate : String = {
            val formater = new java.text.SimpleDateFormat ("yyyy-MM-dd HH:mm:ss")
            return formater.format(date)
        }

    }

    implicit val graphValueWrites = new Writes[graphValue] {
      def writes(gv: graphValue) = Json.obj(
          "date"  -> gv.getDate,
          "value" -> gv.value
          )
    }

    /*
     * Graph value representation for the POST
     */
    case class graphPost (date : Option[java.util.Date],
                           value : Int)

    /*
     * Graph data form
     */
    val graphForm : Form[graphPost] = Form(
        mapping(
            "date"  -> optional(date("dd/MM/yyyy/HH/mm/ss")),
            "value" -> number
        )(graphPost.apply)(graphPost.unapply)
    )

    /*
     * Print the chart of the temperatures
     */
    def printTemp = Action {
        Ok(views.html.graph("Temperature", "temp"))
    }

    def calculate (aux : Int) : Double = {
        var temperature : Double = 0
        var i = 0
        for(i <- 0 to 11){
            temperature += ((aux >> i)&0x01)*Math.pow(2,i-4)
        }
        if (((aux >> 12)&0x01) == 1)
            temperature *= -1;
        return temperature
    }

    /* 
     * Set temp value
     */
    def temp = Action { implicit request =>
        graphForm.bindFromRequest.fold(
            error => BadRequest("Bad argument"),
            data  => {
                var aux = data.value
                var temperature : Double = 0
                var i = 0
                for(i <- 0 to 11){
                    temperature += ((aux >> i)&0x01)*Math.pow(2,i-4)
                }
                if (((aux >> 12)&0x01) == 1)
                    temperature *= -1;
                var tempData = MongoDBObject(
                    "data"  -> "temp",
                    "value" -> temperature,
                    "date"  -> data.date.getOrElse(new java.util.Date())
                )
                rawCollection += tempData
                Ok("Temperature received")
            }
        )
    }

    /*
     * Receive a post for a cry
     */
    def cryPost = Action {implicit request =>
        graphForm.bindFromRequest.fold(
            error => BadRequest("Bad argument"),
            data => {
                var tempData = MongoDBObject(
                    "data"  -> "cry",
                    "value" -> data.value,
                    "date"  -> data.date.getOrElse(new java.util.Date())
                )
                rawCollection += tempData
                Ok("Cry received")
            }
        )
    }

    /*
     * Print the chart of the cries
     */
    def cryGet = Action {
        Ok(views.html.graph("Cries", "cry"))
    }

    /*
     * Receive a post for an activity
     */
    def activityPost = Action {implicit request =>
        graphForm.bindFromRequest.fold(
            error => BadRequest("Bad argument"),
            data => {
                var tempData = MongoDBObject(
                    "data"  -> "activity",
                    "value" -> data.value,
                    "date"  -> data.date.getOrElse(new java.util.Date())
                )
                rawCollection += tempData
                Ok("Activity received")
            }
        )
    }

    /*
     * Print the chart of the activities
     */
    def activityGet = Action {
        Ok(views.html.graph("Activity", "activity"))
    }

    /*
     * Receive a post for an presence
     */
    def presencePost = Action {implicit request =>
        graphForm.bindFromRequest.fold(
            error => BadRequest("Bad argument"),
            data => {
                var tempData = MongoDBObject(
                    "data"  -> "presence",
                    "value" -> data.value,
                    "date"  -> data.date.getOrElse(new java.util.Date())
                )
                rawCollection += tempData
                Ok("Presence received")
            }
        )
    }

    /*
     * Print the chart of the activities
     */
    def presenceGet = Action {
        Ok(views.html.graph("Presence", "presence"))
    }

    /* Gridfs reference */
    val gridfs = GridFS(dataBase)

    /*
     * Test the writting/reading in the database
     */
    def testGridFS = {
        gridfs remove "mongodb_test.ogg"

        /* Save in the database */
        var image = new FileInputStream("public/0233.ogg")
        var id = gridfs(image) { f =>
            f.filename = "mongodb_test.ogg"
        }

        var imageReceived = gridfs.findOne("mongodb_test.ogg")
        imageReceived match {
            case Some(im) => println(im.filename)
            case None     => println("Nothing")
        }

        gridfs remove "mongodb_test.ogg"
    }

    /*
     * Uploads a file in the database
     */
    def upload = Action(parse.multipartFormData) { request =>
        request.body.file("file").map { picture =>
            val filename = picture.filename 
            val contentType = picture.contentType

            var f = new File(s"/tmp/$filename")
            f.delete()
            picture.ref.moveTo(f)
            var image = new FileInputStream(f)

            try {
                gridfs remove filename
                /* Write in the database */
                var id = gridfs(image) { f =>
                    f.filename = filename
                    f.contentType = contentType.getOrElse("image/jpg")
                }
                Ok("File uploaded")
            }catch {
                case _ : Throwable => Ok("May be uploaded")
            }
        }.getOrElse {
            BadRequest("Problem while upload")
        }
    }

    /*
     * Page to send an image
     */
    def sendimage = Action {
        Ok(views.html.image())
    }

    /*
     * Reads an image
     */
    def image (name : String) = Action {
        photoRequest = 0;
        var imageReceived = gridfs.findOne(name)
        imageReceived match {
            case Some(im) => {
                var filename = im.filename.getOrElse("image")
                var file = new File(filename)
                im writeTo file
                Ok.sendFile(file)}
            case None     =>
                Ok("No such image")
        }
    }

    /* Channel to communicate to Kudly */
    val (enumKudly, channelKudly) = Concurrent.broadcast[Array[Byte]]

    /* Channel to communicate to Kudly */
    val (enumParent, channelParent) = Concurrent.broadcast[Array[Byte]]

    /*
     * Streaming function
     */
    def streaming = WebSocket.using[Array[Byte]] { request =>
        streamingRequest = 0;
        /* Just print the received values */
        val in = Iteratee.foreach[Array[Byte]] {
            msg => channelParent push msg
        }
        /* File to send */
        val file = new File("public/bell.wav")
        val sound = new FileInputStream(file)
        /* Enumerator to read sound */
        val dataContent: Enumerator[Array[Byte]] =  audioHeader >>> Enumerator.fromStream(sound)
        /* An enumerator that push in kudly channel */
        val pusher = Iteratee.foreach[Array[Byte]](
            s => channelKudly push s )
        val newIteratee: Future[Iteratee[Array[Byte],Unit]] =
            dataContent(pusher)

        (in, enumKudly)
    }

    val samplesPerFrame: Int = 1
    val frameRate: Int = 8000
    val bitsPerSample: Int = 16

    val bytesPerSamples = ((bitsPerSample+7)/8).toInt

    /* Useful types for header */
    private def IntLittleBytes(i: Int) = Array(
      i     toByte,
      i>>8  toByte,
      i>>16 toByte,
      i>>24 toByte
    )

    private def ShortLittleBytes(i: Short) = Array(
      i    toByte,
      i>>8 toByte
    )

    /* Header for the streaming */
    lazy val header: Array[Byte] = {
        val riff = "RIFF".getBytes ++
                   /* Maximum chunk size (we are streaming here */
                   IntLittleBytes(0x7fffffff) ++
                   "WAVE".getBytes

        val fmt =  "fmt ".getBytes ++
                   /* Subchunk1Size for PCM = 16 */
                   IntLittleBytes(16) ++
                   /* AudioFormat for PCM = 1 */
                   ShortLittleBytes(1) ++
                   ShortLittleBytes(samplesPerFrame toShort) ++
                   IntLittleBytes(frameRate) ++
                   IntLittleBytes(frameRate*samplesPerFrame*bytesPerSamples) ++
                   ShortLittleBytes(samplesPerFrame*bytesPerSamples toShort) ++
                   ShortLittleBytes(bitsPerSample toShort);

        val data = "data".getBytes ++
                   IntLittleBytes(0x7fffffff);

        riff ++ fmt ++ data;
    }

    /* The enumerator of the Header */
    val audioHeader = Enumerator(header)

    /* Streams the sound to the parents */
    def toParent = Action {
        streamingRequest = 1;
        Ok.chunked(audioHeader >>> enumParent).
            withHeaders( (CONTENT_TYPE, "audio/wav"),
                         (CACHE_CONTROL, "no-cache"))
    }

    /*
     * Update file of configuration
     */
    def configFile = Action {
        Ok(<config>
            <stream state={streamingRequest.toString}/>
            <photo state={photoRequest.toString}/>
        </config>)
    }

    /*
     * Asks for a photo
     */
    def cameraRequest = Action {
        photoRequest = 1;
        Ok("Photo requested")
    }

    /*
     * Return a JSON
     */

    def dataJSON (name : String, db : String) = Action {
        var list : List[graphValue] = List()
        rawCollection.find("data" $eq db).foreach(
            data =>
                list = graphValue(
                    data.getAs[java.util.Date]("date").getOrElse(new java.util.Date()),
                    data.getAs[Int]("value").getOrElse(0)) :: list )
        val json : JsValue = Json.obj(
            "type" -> "serial",
            "pathToImages" -> "http://cdn.amcharts.com/lib/3/images/",
            "categoryField" -> "date",
            "dataDateFormat" -> "YYYY-MM-DD HH:NN",
            "categoryAxis" -> Json.obj(
                "minPeriod" -> "mm",
                "parseDates" -> true),
            "chartCursor" -> Json.obj(
                "categoryBalloonDateFormat" -> "JJ:NN"
            ),
            "chartScrollbar" -> Json.obj(),
            "trendLines" -> Json.arr(),
            "graphs" -> Json.arr(
                Json.obj(
                    "bullet" -> "square",
                    "id" -> "AmGraph-2",
                    "title" -> name,
                    "valueField" -> "column-1"
                    )
            ),
            "guides" -> Json.arr(),
            "valueAxes" -> Json.arr(
                Json.obj(
                    "id" -> "v1",
                    "axisAlpha" -> 0,
                    "position" -> "left"
                    )
                ),
            "balloon" ->
                Json.obj(
                    "borderThickness" -> 1,
                    "shadowAlpha" -> 0
                ),
            "graphs" -> Json.arr(
                Json.obj(
                    "id" -> "g1",
                    "type" -> "step",
                    "bullet" -> "round",
                    "bulletBorderAlpha" -> 1,
                    "bulletColor" -> "#FFFFFF",
                    "bulletSize" -> 5,
                    "hideBulletsCount" -> 50,
                    "lineColor" -> "#81ABD9",
                    "lineThickness" -> 2,
                    "title" -> "red line",
                    "useLineColorForBulletBorder" -> true,
                    "valueField" -> "value",
                    "balloonText" -> "<div style='margin:5px; font-size:19px;'><span style='font-size:13px;'>[[category]]</span><br>[[value]]</div>"
                )
            ),
            "chartScrollbar" ->
                Json.obj(
                    "graph" -> "g1",
                    "scrollbarHeight" -> 80,
                    "backgroundAlpha" -> 0,
                    "selectedBackgroundAlpha" -> 0.1,
                    "selectedBackgroundColor" -> "#888888",
                    "graphFillAlpha" -> 0,
                    "graphLineAlpha" -> 0.5,
                    "selectedGraphFillAlpha" -> 0,
                    "selectedGraphLineAlpha" -> 1,
                    "autoGridCount" -> true,
                    "color" -> "#AAAAAA"
                ),
            "chartCursor" ->
                Json.obj(
                    "pan" -> true,
                    "valueLineEnabled" -> true,
                    "valueLineBalloonEnabled" -> true,
                    "cursorAlpha" -> 0,
                    "valueLineAlpha" -> 0.2
                ),
            "categoryField" -> "date",
            "categoryAxis" ->
                Json.obj(
                    "minPeriod" -> "ss",
                    "parseDates" -> true,
                    "dashLength" -> 1,
                    "minorGridEnabled" -> true,
                    "position" -> "top"
                ),
            "export" ->
                Json.obj(
                    "enabled" -> true,
                    "libs" ->
                    Json.obj(
                        "path" -> "http://www.amcharts.com/lib/3/plugins/export/libs/"
                    )
                ),
            "dataProvider" -> list.sortWith((x,y) => x.date.before(y.date))
            );
       Ok(json)
    }
}
