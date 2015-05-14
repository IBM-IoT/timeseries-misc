/**************************************************************************
 *
 * Licensed Materials - Property of IBM Corporation
 *
 * Restricted Materials of IBM Corporation
 *
 * IBM Informix JDBC Driver
 * (c) Copyright IBM Corporation 1998, 2013  All rights reserved.
 *
 ****************************************************************************/
/***************************************************************************
 *
 *  Title:         BulkInsert.java
 *
 *  Description: This program demonstrates the use of IFX_USEPUT with single
 *               insert statement in batch execution mode.
 *
 *  Instructions to run the program
 *
 *  1. Use CreateDB to create the Database testdb if not already done.
 *           java CreateDB 'jdbc:informix-sqli:
 *           //myhost:1533:user=<username>;password=<password>'
 *  2. Run the program by:
 *          java BulkInsert
 *          'jdbc:informix-sqli://myhost:1533:
 *          user=<username>;password=<password>'
 *  3. Expected Result 
 * URL = "jdbc:informix-sqli://myhost:1533/testDB:user=<username>;password=<password>"
 * Trying to insert data using PreparedStatement ...
 * Number of records inserted (should be 1000):
 *  records inserted = 1000

 *
 ***************************************************************************
 */
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;
import java.util.StringTokenizer;

public class BulkInsert {
    
    public static void main(String[] args) {


        String url =null;
        Connection conn = null;
        Statement stmt = null;
        String newUrl = "";
        Integer batchSize = 1000;
        Integer batchCount = 0;
        String tstamp; 
        Integer pmu_id=0;
      
        if (args.length == 00) {
           System.out.println("Usage: BulkInsert <id>"); 
           return;
        }
        pmu_id = Integer.parseInt(args[0]); 

        System.out.println("Load for id: " + pmu_id);


/*
 *   Read Metadata from properties File
 */ 

try {
        File propFilename=new File("bi.prop");
        FileInputStream fi = new FileInputStream(propFilename);
        Properties prop = new Properties();

        prop.load(fi);
       
        url = prop.getProperty("url");
        batchSize = Integer.parseInt(prop.getProperty("batch.size"));

    } catch (FileNotFoundException e) {
          e.printStackTrace();
    } catch (IOException e) {
          e.printStackTrace();
    }

    //System.out.println("URL: " + url);
    //System.out.println("batchSize: " + batchSize);

       
/*
 *   Load JDBC Driver
 */

        try {
            Class.forName("com.informix.jdbc.IfxDriver");
        } catch (Exception e) {
            System.out.println("FAILED: failed to load Informix JDBC driver.");
        }
        
        try {
            Properties pr = new Properties();
            pr.put("IFX_USEPUT", "1");
            conn = DriverManager.getConnection(url, pr);
        } catch (SQLException e) {
            System.out.println("FAILED: failed to connect!");
        }

        //System.out.println("Trying to insert data using PreparedStatement ...");
        try {
            PreparedStatement pst = conn.prepareStatement("insert into pmu_large_vti values (?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?)");

       batchCount=0;
       for (int min=0; min <= 59; min++) {
          for (int sec=0; sec <= 59; sec++) {
            for (int ms = 0; ms <= 59; ms++) {
               String prec = String.format("%.5f",ms/60.0);
               tstamp = String.format("2015-01-01 08:%02d:%02d.%s", min,sec,prec.substring(2,7));
               //System.out.println("TS: " + tstamp);




               pst.setInt(1, pmu_id);
               pst.setString(2, tstamp);
               for( int x=3; x<= 17; x++) {
                  pst.setDouble(x, 3.4);
               }
               pst.addBatch();
               batchCount++;

               //System.out.println("Count: " + batchCount);
               if (batchCount.equals(batchSize)) {
                  //System.out.println("Batch");
                  pst.executeBatch();
                  batchCount=0;
               }

            } // end ms
         } // End sec
      } // Min sec



        } catch (SQLException e) {
            System.out.println("FAILED to Insert into tab: " + e.toString());
        }
        



    
    }
    
}
