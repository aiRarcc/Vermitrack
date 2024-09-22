import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:percent_indicator/circular_percent_indicator.dart';
import 'package:http/http.dart' as http;
import 'dart:convert';

class temperature extends StatelessWidget {
  const temperature({super.key});

  // Stream for fetching temperature data
  Stream<double> fetchtemperature() async* {
    final url =
        Uri.https('verminn-dde78-default-rtdb.firebaseio.com', 'DHT_11.json');
    while (true) {
      try {
        final response = await http.get(url);
        if (response.statusCode == 200) {
          final jsonData = json.decode(response.body);
          double temperatureValue =
              double.parse(jsonData['Temperature'].toString());
          yield temperatureValue; // Yield the temperature value
        } else {
          yield 0.0; // Error handling
        }
      } catch (e) {
        print('Error: $e');
        yield 0.0; // Default value in case of error
      }
      await Future.delayed(
          const Duration(seconds: 5)); // Adjust the delay as needed
    }
  }

  @override
  Widget build(BuildContext context) {
    double displayWidth = MediaQuery.of(context).size.width;

    currentTColor(double tr) {
      if (tr >= 1 && tr < 30) {
        return Colors.blue;
      }
      if (tr >= 30 && tr < 40) {
        return Colors.green;
      } else {
        return Colors.red;
      }
    }

    return StreamBuilder<double>(
      stream: fetchtemperature(), // Listen to the temperature stream
      builder: (context, snapshot) {
        if (snapshot.connectionState == ConnectionState.waiting) {
          return const CircularProgressIndicator(); // Loading indicator
        } else if (snapshot.hasError) {
          return Text('Error: ${snapshot.error}'); // Handle errors
        } else if (snapshot.hasData) {
          double tv =
              snapshot.data! / 100; // Assuming you want to use percentage
          double tr = snapshot.data!;
          String td = '${tr.toStringAsFixed(1)} °C';

          return Container(
            padding: const EdgeInsets.all(10.0),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                Container(
                  // TEMPERATURE
                  padding: const EdgeInsets.all(10),
                  width: displayWidth * .89,
                  height: displayWidth * .70,
                  decoration: BoxDecoration(
                    color: const Color(0xffffffff).withOpacity(.55),
                    border: Border.all(
                      color: const Color(0xff013e3e),
                    ),
                    borderRadius: const BorderRadius.all(
                      Radius.circular(25),
                    ),
                  ),
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.spaceAround,
                    children: [
                      Row(
                        children: [
                          Image.asset(
                            'lib/uicons/temp.png',
                            height: displayWidth * .1,
                          ),
                          Text(
                            "Temperature",
                            style: GoogleFonts.nunito(
                                color: const Color(0xff013e3e),
                                fontSize: displayWidth * .04,
                                fontWeight: FontWeight.bold),
                          ),
                        ],
                      ),
                      CircularPercentIndicator(
                        animation: true,
                        animateFromLastPercent: true,
                        animationDuration: 1000,
                        backgroundColor: Colors.transparent,
                        radius: displayWidth * .25,
                        lineWidth: displayWidth * .05,
                        progressColor: currentTColor(tr),
                        startAngle: 230,
                        circularStrokeCap: CircularStrokeCap.round,
                        percent: tv, // PERCENTAGE
                        center: Text(
                          td, // NUM DISPLAY
                          style: TextStyle(
                            fontSize: displayWidth * .06,
                            fontWeight: FontWeight.w500,
                            color: const Color(0xff013e3e),
                          ),
                        ),
                      ),
                    ],
                  ),
                ),
              ],
            ),
          );
        } else {
          return const Text('No data available');
        }
      },
    );
  }
}
