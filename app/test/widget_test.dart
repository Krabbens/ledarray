// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.
import 'package:flutter_test/flutter_test.dart';

import 'package:conn_app/app.dart';
import 'package:conn_app/views/pre_connection_view.dart';


void main() {
  testWidgets('App should start with PreConnectionRouter', (WidgetTester tester) async {
    // Build our app and trigger a frame.
    await tester.pumpWidget(const App());

    // Verify that PreConnectionRouter is the first widget
    expect(find.byType(PreConnectionView), findsOneWidget);
  });
}
