import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';

class Light extends StatelessWidget {
  const Light({super.key});

  @override
  Widget build(BuildContext context) {
    double displayWidth = MediaQuery.of(context).size.width;
    //                                                                    LIGHT
    bool ls = true;
    var lsc = ls.toString();
    String lss = '';
    if (lsc == 'true') {
      lss = 'In Operation';
    } else {
      lss = 'On Stanby';
    }
    return Container(
      padding: const EdgeInsets.all(5.0),
      child: Container(
        padding: const EdgeInsets.all(15),
        width: displayWidth * .89,
        height: displayWidth * .20,
        decoration: BoxDecoration(
          color: const Color(0xffffffff).withOpacity(.55),
          border: Border.all(
            color: const Color(0xff013e3e),
          ),
          borderRadius: const BorderRadius.all(
            Radius.circular(25),
          ),
        ),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Row(
              children: [
                Image.asset(
                  'lib/uicons/light.png',
                  height: displayWidth * .1,
                ),
                Text(
                  "  Light",
                  style: GoogleFonts.nunito(
                    color: const Color(0xff013e3e),
                    fontSize: displayWidth * .04,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
            Text(
              lss, //                                                         STATUS
              style: GoogleFonts.quicksand(
                color: const Color(0xff013e3e),
                fontSize: displayWidth * .04,
                fontWeight: FontWeight.w500,
              ),
            ),
          ],
        ),
      ),
    );
  }
}
