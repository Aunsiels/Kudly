package controllers

import play.api._
import play.api.mvc._
import play.api.data._
import play.api.data.Forms._
import com.mongodb.casbah.Imports._
import play.api.libs.json._

object Graph extends Controller {

    /* Database */
    val dataBase = MongoClient(
        MongoClientURI(
            "mongodb://kudly:19071993@ds041168.mongolab.com:41168/kudly"
        )
    ).getDB("kudly")

    /* Collection of raw data */
    val rawCollection = dataBase("Raw")

    /*
     * Graph value representation
     */
    case class graphValue (date : java.util.Date,
                           value : Int) {
        /* Return a format Date */
        def getDate : String = {
            val formater =
                new java.text.SimpleDateFormat ("yyyy-MM-dd HH:mm:ss")
            formater.format(date)
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
        if (((aux >> 12)&0x01) == 1){
            temperature *= -1
        }
        temperature
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
                if (((aux >> 12)&0x01) == 1){
                    temperature *= -1
                }
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

    /*
     * Return a JSON
     */
    def dataJSON (name : String, db : String) = Action {
        var list : List[graphValue] = List()
        if (db == "temp") {
            rawCollection.find("data" $eq db).foreach(
                    data =>
                    list = graphValue(
                        data.getAs[java.util.Date]("date").getOrElse(new java.util.Date()),
                        data.getAs[Int]("value").getOrElse(
                            data.getAs[Double]("value").getOrElse(26.0).toInt)) ::
                        list )
        }else {
            rawCollection.find("data" $eq db).foreach(
                    data =>
                    list = graphValue(
                        data.getAs[java.util.Date]("date").getOrElse(new java.util.Date()),
                        data.getAs[Int]("value").getOrElse(0)) :: list )
        }
        val typeData = if(name == "Activity") "step" else "line"
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
                    "type" -> typeData,
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
