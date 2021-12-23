import java.io.*;
import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Scanner;

public class Test {
    private static enum Type {
        WORD {
            public boolean compare(String a, String b) {
                return a.trim().equals(b.trim());
            }
        },
        NUMBER {
            public boolean compare(String a, String b) {
                boolean ret = false;
                try {
                    BigDecimal d1 = new BigDecimal(Double.parseDouble(a));
                    BigDecimal d2 = new BigDecimal(Double.parseDouble(b));
                    ret = d1.equals(d2);
                } catch (Exception e) {

                }
                return ret;
            }
        };

        public boolean compare(String a, String b) {
            return false;
        }
    }

    private static class Value {
        private Type type;
        private String value;
        private int mark;
        private String comment;

        public void read(Scanner in) {
            String line = in.nextLine();
            String[] a = line.split(",");
            this.type = Type.valueOf(a[0].toUpperCase());
            this.value = a[1];
            this.mark = Integer.valueOf(a[2].trim());
            this.comment = a[3];
        }

        public boolean judge(String line) {
            return type.compare(line, value);
        }

        public int getMark() {
            return mark;
        }

        public String getComment() {
            return comment;
        }

        @Override
        public String toString() {
            return type + "," + value + "," + mark + "," + comment;
        }
    }

    public static void main(String[] args) throws IOException {
        // prepare standard output and input
        ArrayList<Value> alValue = new ArrayList<>();
        Scanner sout = new Scanner(new FileInputStream("stdout"));
        while (sout.hasNext()) {
            Value v = new Value();
            v.read(sout);
            alValue.add(v);
        }
        sout.close();

        PrintWriter resultout = new PrintWriter(new OutputStreamWriter(new FileOutputStream(args[0])));
        // execution
        int mark = 0;
        try {
            var process = new ProcessBuilder().command("sh", "run.sh").start();
            inheritIO(process.getErrorStream(), System.err);
            var pin = process.getInputStream();
            Scanner in = new Scanner(pin);
            Iterator<Value> it = alValue.iterator();
            while (in.hasNext() && it.hasNext()) {
                String line = in.nextLine();
                Value v = it.next();
                if (v.judge(line)) {
                    mark += v.getMark();
                    resultout.println(v.comment + ": PASS");
                } else {
                    resultout.println(v.comment + ": FAIL");
                }
            }
            in.close();
        } catch (Throwable e) {
            e.printStackTrace(resultout);
        }
        var scoreout = new PrintWriter(new OutputStreamWriter(new FileOutputStream("score")));

        scoreout.println(mark);
        resultout.close();
        scoreout.close();
    }

    private static void inheritIO(final InputStream src, final PrintStream dest) {
        new Thread(new Runnable() {
            public void run() {
                Scanner sc = new Scanner(src);
                while (sc.hasNextLine()) {
                    dest.println(sc.nextLine());
                }
            }
        }).start();
    }
}

