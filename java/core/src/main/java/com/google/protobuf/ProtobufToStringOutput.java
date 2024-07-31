package com.google.protobuf;

/**
 * ProtobufToStringOutput control the output format of {@link Message#toString()}. Specifically, for
 * the Runnable object passed to `callWithDebugFormat` and `callWithTextFormat`, Message.toString()
 * will always output the specified format unless ProtobufToStringOutput is used again to change the
 * output format.
 */
public final class ProtobufToStringOutput {
  private enum OutputMode {
    DEBUG_FORMAT,
    TEXT_FORMAT
  }

  private static final ThreadLocal<OutputMode> outputMode =
      new ThreadLocal<OutputMode>() {
        @Override
        protected OutputMode initialValue() {
          return OutputMode.TEXT_FORMAT;
        }
      };

  private ProtobufToStringOutput() {}

  private static OutputMode setOutputMode(OutputMode newMode) {
    OutputMode oldMode = outputMode.get();
    outputMode.set(newMode);
    return oldMode;
  }

  private static void callWithSpecificFormat(Runnable impl, OutputMode mode) {
    OutputMode oldMode = setOutputMode(mode);
    try {
      impl.run();
    } finally {
      var unused = setOutputMode(oldMode);
    }
  }

  // @RestrictedApi(
  //    explanation = "Restricted API to change the output mode of Protobuf Message.toString")
  public static void callWithDebugFormat(Runnable impl) {
    callWithSpecificFormat(impl, OutputMode.DEBUG_FORMAT);
  }

  // @RestrictedApi(
  //    explanation = "Restricted API to change the output mode of Protobuf Message.toString")
  public static void callWithTextFormat(Runnable impl) {
    callWithSpecificFormat(impl, OutputMode.TEXT_FORMAT);
  }

  public static boolean shoudlOuputDebugFormat() {
    return outputMode.get() == OutputMode.DEBUG_FORMAT;
  }
}
